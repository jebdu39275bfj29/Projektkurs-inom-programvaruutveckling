#include "GameController.h"
#include "GameModel.h"
#include "GameView.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdbool.h>

#define MOVE_SPEED 2.0f     
#define THRESHOLD 5.0f  

static bool handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return false;
        }
    }
    return true;
}

int startGameLoop() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        SDL_Log("IMG_Init Error: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }

    if (!initializeTextSystem()) {
        SDL_Log("TTF Error:\n");
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Football Simulation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    GameTextures textures = loadAllTextures(renderer);

    if (!textures.playerTexture || !textures.grassTexture || !textures.coachTexture) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    GameModel model;
    initializeModel(&model, textures.coachTexture);
    model.ball.texture = textures.ballTexture;

    model.ball.texture = textures.ballTexture;

    bool running = true;

    while (running) {
        running = handleEvents();

        updatePassingLogic(&model);
        renderGame(renderer, textures.playerTexture, textures.grassTexture, &model);
        SDL_Delay(16);
    }

    cleanupModel(&model);
    SDL_DestroyTexture(textures.playerTexture);
    SDL_DestroyTexture(textures.grassTexture);
    SDL_DestroyTexture(textures.coachTexture);
    SDL_DestroyTexture(model.ball.texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}


void updatePassingLogic(struct GameModel* model) {
    int from = model->passOrder[model->step % PLAYER_COUNT];
    int to   = model->passOrder[(model->step + 1) % PLAYER_COUNT];

    float targetX = model->players[to].x;
    float targetY = model->players[to].y;

    // 1) Flytta spelare "from" mot "to"
    movePlayerTowards(&model->players[from], targetX, targetY, MOVE_SPEED, model);

    // 2) Placera bollen framför spelare "from"
    float dx = model->players[from].x - model->ball.x;
    float dy = model->players[from].y - model->ball.y;
    float dist = sqrtf(dx*dx + dy*dy);

    // T.ex. rulla bollen med hastighet proportionell mot dist
    if (dist > 0.1f) {
        // Rulla bollen proportionellt mot hur långt den förflyttar sig
        float rollSpeed = 5.0f; // testvärde: justera så bollens rotation känns bra
        model->ball.angle += rollSpeed;
        if (model->ball.angle > 360.0f) {
            model->ball.angle -= 360.0f;
        }
    }

    // Här kan du lägga en offset så bollen hamnar “framför” spelaren.
    // T.ex. 20 pixlar i rörelsens riktning
    float offset = 20.0f;
    float angleRad = atan2f( (targetY - model->players[from].y),
                             (targetX - model->players[from].x) );
    float offsetX = cosf(angleRad) * offset;
    float offsetY = sinf(angleRad) * offset;

    model->ball.x = model->players[from].x + offsetX;
    model->ball.y = model->players[from].y + offsetY;

    // 3) Kolla om passningen är “framme”
    float coachTargetX = targetX - 50;
    float coachTargetY = targetY - 30;

    coachTargetX = fmax(model->grass.x, fmin(coachTargetX, model->grass.x + model->grass.width));
    coachTargetY = fmax(model->grass.y, fmin(coachTargetY, model->grass.y + model->grass.height));

    //movePlayerTowards(&model->coach, coachTargetX, coachTargetY, MOVE_SPEED, model);
    steerCoachNoStuck(model, coachTargetX, coachTargetY, MOVE_SPEED);


    if (fabs(model->players[from].x - targetX) < THRESHOLD &&
        fabs(model->players[from].y - targetY) < THRESHOLD) {
        model->step = (model->step + 1) % PLAYER_COUNT;
    }
}


/////////////////////////----- Collision Detection -----///////////////////////////////////
void steerCoachNoStuck(GameModel* model, float targetX, float targetY, float speed)
{
    // 1) Beräkna rak mål-vektor
    float dx = targetX - model->coach.x;
    float dy = targetY - model->coach.y;
    float dist = sqrtf(dx*dx + dy*dy);

    if (dist < 1.0f) {
        // Coach är nära målet
        model->coach.animationState = IDLE;
        model->coach.frame = 0;
        return;
    }

    float baseAngle = atan2f(dy, dx); // Radianer
    float vx = cosf(baseAngle);
    float vy = sinf(baseAngle);

    // 2) Lägg på repulsion (som tidigare)
    float repulsionRange = 150.0f; 
    float repulsionStrength = 100.0f;

    float cx = model->coach.x + PLAYER_SIZE/2.0f;
    float cy = model->coach.y + PLAYER_SIZE/2.0f;

    // 1) Hitta närmaste spelare
    float nearestDist = 999999.0f;
    int nearestIndex = -1;

    for (int i = 0; i < PLAYER_COUNT; i++) {
        float px = model->players[i].x + PLAYER_SIZE/2.0f;
        float py = model->players[i].y + PLAYER_SIZE/2.0f;
        float dxP = cx - px;
        float dyP = cy - py;
        float distP = sqrtf(dxP*dxP + dyP*dyP);

        if (distP < nearestDist) {
            nearestDist = distP;
            nearestIndex = i;
        }
    }

    // 2) Bara om närmaste spelaren är inom repulsionRange => räkna ut repulsion
    if (nearestIndex >= 0 && nearestDist < repulsionRange && nearestDist > 0.001f) {
        float scale = (repulsionRange - nearestDist) / repulsionRange;
        float dxP = cx - (model->players[nearestIndex].x + PLAYER_SIZE/2.0f);
        float dyP = cy - (model->players[nearestIndex].y + PLAYER_SIZE/2.0f);
        float rx = (dxP / nearestDist) * scale * (repulsionStrength / 100.0f);
        float ry = (dyP / nearestDist) * scale * (repulsionStrength / 100.0f);

        vx += rx;
        vy += ry;
    }


    // Normalisera vektorn
    float finalLen = sqrtf(vx*vx + vy*vy);
    if (finalLen > 0.0001f) {
        vx /= finalLen;
        vy /= finalLen;
    }
    // Multiplicera med speed
    vx *= speed;
    vy *= speed;

    // 3) Nu ska vi testa upp till ex. ±30 grader i små steg
    //    för att hitta en vinkel som inte krockar med spelare
    //    Börja med “mainAngle” = atan2f(vy, vx), testa ±2°, ±4°, ...
    float mainAngle = atan2f(vy, vx);
    const float angleStep = 0.0349f; // ~2 grader i radianer
    const int maxSteps = 15;        // vi testar ±(15*2)=30 grader
    bool foundMove = false;

    float bestNx = model->coach.x;  
    float bestNy = model->coach.y;

    for (int stepI = 0; stepI <= maxSteps && !foundMove; stepI++) {
        // Vi går varannan gång vänster, varannan gång höger: stepI/2 * angleStep
        // stepI=0 => 0°, stepI=1 => +2°, stepI=2 => -2°, stepI=3 => +4°, ...
        int half = stepI / 2;
        bool isOdd = (stepI % 2 == 1);
        float tryAngle = mainAngle + (isOdd ? (half+1)*angleStep : -half*angleStep);

        float testVx = cosf(tryAngle) * speed;
        float testVy = sinf(tryAngle) * speed;

        float testNx = model->coach.x + testVx;
        float testNy = model->coach.y + testVy;

        // Kolla kollision med spelare
        float newCx = testNx + PLAYER_SIZE/2.0f;
        float newCy = testNy + PLAYER_SIZE/2.0f;

        float coachRad = model->coachDetectionRadius;
        float playerRad = PLAYER_SIZE/2.0f;
        bool collide = false;

        for (int p = 0; p < PLAYER_COUNT; p++) {
            float px = model->players[p].x + PLAYER_SIZE/2.0f;
            float py = model->players[p].y + PLAYER_SIZE/2.0f;
            float distToPlayer = sqrtf((newCx - px)*(newCx - px) + (newCy - py)*(newCy - py));
            float minDist = coachRad + playerRad;
            if (distToPlayer < minDist) {
                collide = true;
                break;
            }
        }

        if (!collide) {
            // Bra, vi hittade en vinkel som inte kolliderar
            foundMove = true;
            bestNx = testNx;
            bestNy = testNy;
        }
    }

    // Om vi fortfarande inte hittade en vinkel => coachen står still
    float oldX = model->coach.x;
    float oldY = model->coach.y;

    float finalX = foundMove ? bestNx : oldX;
    float finalY = foundMove ? bestNy : oldY;

    // 4) Sätt coachens position, vinkel, animation
    float moveDist = sqrtf((finalX - oldX)*(finalX - oldX) + (finalY - oldY)*(finalY - oldY));
    if (moveDist > 0.1f) {
        model->coach.x = finalX;
        model->coach.y = finalY;

        float aDeg = atan2f(finalY - oldY, finalX - oldX) * (180.0f / M_PI);
        if (aDeg < 0) aDeg += 360;
        model->coach.angle = aDeg;
        model->coach.animationState = RUN;
    } else {
        // Står i princip still => men vi kan låta coachen 'springa' på stället:
        model->coach.animationState = RUN;
    }

    // Animationsuppdatering
    Uint32 now = SDL_GetTicks();
    if (now - model->coach.lastFrameTime > FRAME_DELAY) {
        int frameCount = animationFrameCounts[model->coach.animationState];
        model->coach.frame = (model->coach.frame + 1) % frameCount;
        model->coach.lastFrameTime = now;
    }
}
