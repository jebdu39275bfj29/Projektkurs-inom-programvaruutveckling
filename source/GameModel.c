#include "GameModel.h"
#include <math.h>

Player squarePlayers[SQUARE_PLAYER_COUNT];

const int animationFrameCounts[ANIMATION_COUNT] = {
    4, 8, 10, 6, 8, 9
};

void cleanupModel(struct GameModel* model) {
    if (model->coachTexture) {
        SDL_DestroyTexture(model->coachTexture);
        model->coachTexture = NULL;
    }
}

void initSquarePlayers() {
    int gap = 400;

    // Justera dessa värden för att flytta positionen
    int shiftX = -40;  // flytta vänster
    int shiftY = -50;   // flytta uppåt

    // Center point of the square
    int centerX = WINDOW_WIDTH / 2 + shiftX;
    int centerY = WINDOW_HEIGHT / 2 + shiftY;

    // Offset to spread players around the center
    int offset = gap / 2;

    // Upper left
    squarePlayers[0] = (Player){
        .x = centerX - offset,
        .y = centerY - offset,
        .angle = 0,
        .state = IDLE,
        .animationState = IDLE,
        .frame = 0,
        .lastFrameTime = SDL_GetTicks()
    };

    // Lower left
    squarePlayers[1] = (Player){
        .x = centerX - offset,
        .y = centerY + offset,
        .angle = 0,
        .state = IDLE,
        .animationState = IDLE,
        .frame = 0,
        .lastFrameTime = SDL_GetTicks()
    };

    // Upper right
    squarePlayers[2] = (Player){
        .x = centerX + offset,
        .y = centerY - offset,
        .angle = 0,
        .state = IDLE,
        .animationState = IDLE,
        .frame = 0,
        .lastFrameTime = SDL_GetTicks()
    };

    // Lower right
    squarePlayers[3] = (Player){
        .x = centerX + offset,
        .y = centerY + offset,
        .angle = 0,
        .state = IDLE,
        .animationState = IDLE,
        .frame = 0,
        .lastFrameTime = SDL_GetTicks()
    };
}



void initializeModel(struct GameModel* model, SDL_Texture* coachTexture, SDL_Texture* ballTexture)
 {
    memset(model, 0, sizeof(GameModel));  // nollställer ALLT
    model->grass.x = 10;
    model->grass.y = 10;
    model->grass.width = 780;
    model->grass.height = 580;
    model->grass.texture = NULL;
    model->currentPage = PAGE_MAIN;

    initSquarePlayers();

    int offsetX = 10;
    int offsetY = 10;
    Uint32 startTime = SDL_GetTicks();

    model->players[0] = (Player){ model->grass.x, model->grass.y - offsetY, 0, startTime, IDLE, 0, 0, 0, 0, 0, 0, 0, BEHAVIOR_WAIT_FOR_BALL };
    model->players[1] = (Player){ model->grass.x + model->grass.width - 10 * offsetX, model->grass.y, 0, startTime, IDLE, 0, 0, 0, 0, 0, 0, 0, BEHAVIOR_CHAIN_MOVE };
    model->players[2] = (Player){ model->grass.x - offsetX, model->grass.y + model->grass.height - 10 * offsetY, 0, startTime, IDLE, 0, 0, 0, 0, 0, 0, 0, BEHAVIOR_IDLE };
    model->players[3] = (Player){ model->grass.x + model->grass.width - 10 * offsetX, model->grass.y + model->grass.height - 10 * offsetY, 0, startTime, IDLE, 0, 0, 0, 0, 0, 0, 0, BEHAVIOR_CHAIN_MOVE };
    model->players[4] = (Player){ model->grass.x + model->grass.width / 2, model->grass.y, 0, startTime, IDLE, 0, 0, 0, 0, 0, 0, 0, BEHAVIOR_WAIT_FOR_BALL };
    model->players[5] = (Player){ model->grass.x + (model->grass.width + 2 * offsetX) / 2, model->grass.y + model->grass.height - 11 * offsetY, 0, startTime, IDLE, 0, 0, 0, 0, 0, 0, 0, BEHAVIOR_IDLE };


    model->coach = (Player){
        model->grass.x + model->grass.width / 2,
        model->grass.y + model->grass.height / 2,
        0, startTime, IDLE,
        0, 0, 0, 0, 0, 0, 0,
        BEHAVIOR_IDLE
    };

    model->coachTexture = coachTexture;
    model->coachDetectionRadius = 60.0f;
    model->coachManual = false;
    model->coachTargetX = model->coach.x;
    model->coachTargetY = model->coach.y;

    int order[PLAYER_COUNT] = {4, 1, 3, 5, 2, 0}; 

    int centerX = WINDOW_WIDTH / 2;
    int topY = 60;
    int bottomY = 400;

    model->trianglePlayers[0] = (Player){ // nedre vänster
        .x = 140,
        .y = 420,
        .state = IDLE,
        .hasBall = 1,
        .angle = 0,
        .animationState = IDLE,
        .frame = 0,
        .lastFrameTime = SDL_GetTicks()
    };

    model->trianglePlayers[1] = (Player){ // nedre höger
        .x = 560,
        .y = 420,
        .state = IDLE,
        .hasBall = 0,
        .angle = 0,
        .animationState = IDLE,
        .frame = 0,
        .lastFrameTime = SDL_GetTicks()
    };

    model->trianglePlayers[2] = (Player){ // övre spets
        .x = 380,
        .y = 100,
        .state = IDLE,
        .hasBall = 0,
        .angle = 0,
        .animationState = IDLE,
        .frame = 0,
        .lastFrameTime = SDL_GetTicks()
    };

    model->trianglePlayers[3] = (Player){ // springaren (startar vid nedre vänster)
        .x = 140,
        .y = 420,
        .state = RUN,
        .hasBall = 0,
        .angle = 0,
        .animationState = RUN,
        .frame = 0,
        .lastFrameTime = SDL_GetTicks()
    };

    model->triangleCoach = (Player){
        .x = WINDOW_WIDTH / 2,
        .y = 220,
        .angle = 0,
        .state = IDLE,
        .animationState = IDLE,
        .frame = 0,
        .lastFrameTime = SDL_GetTicks()
    };


    // Passordning: springaren -> punkt1 -> punkt2 -> punkt3
    model->trianglePassOrder[0] = 0; // vänster spets
    model->trianglePassOrder[1] = 1; // höger spets
    model->trianglePassOrder[2] = 2; // övre spets
    model->trianglePassOrder[3] = 3; // bottenmitten
    model->triangleStep = 0;

    model->triangleCoachManual = false;
    model->triangleCoachTargetX = model->triangleCoach.x;
    model->triangleCoachTargetY = model->triangleCoach.y;

    for (int i = 0; i < PLAYER_COUNT; i++) {
        model->passOrder[i] = order[i];
        int index = order[i];
        model->players[index].originalX = model->players[index].x;
        model->players[index].originalY = model->players[index].y;
        model->players[index].targetX = model->players[index].x;
        model->players[index].targetY = model->players[index].y;
        model->players[index].hasBall = 0;

        model->cornerX[i] = model->players[index].x;
        model->cornerY[i] = model->players[index].y;
    }

    model->balls[0].x = model->players[order[0]].x;
    model->balls[0].y = model->players[order[0]].y;
    model->balls[0].attachedPlayer = order[0];
    model->balls[0].state = ATTACHED;
    model->balls[0].frame = 0;
    model->balls[0].lastFrameTime = SDL_GetTicks();
    model->balls[0].angle = 0;
    model->balls[0].texture = NULL;
    model->players[order[0]].hasBall = 1;

    model->balls[1].x = model->players[order[3]].x;
    model->balls[1].y = model->players[order[3]].y;
    model->balls[1].attachedPlayer = order[3];
    model->balls[1].state = ATTACHED;
    model->balls[1].frame = 0;
    model->balls[1].lastFrameTime = SDL_GetTicks();
    model->balls[1].angle = 0;
    model->balls[1].texture = NULL;
    model->players[order[3]].hasBall = 1;

    model->step = 0;
    model->lastPassTime = SDL_GetTicks();


    model->squarePassIndex = 0;
    model->lastSquarePassTime = SDL_GetTicks();
    model->squareBall.texture = ballTexture;
    model->squareBall.frame = 0;
    model->squareBall.lastFrameTime = SDL_GetTicks();
    model->squareBall.state = ATTACHED;
    model->squareBall.attachedPlayer = 0;


    model->squareBallX = squarePlayers[0].x + PLAYER_SIZE / 2;
    model->squareBallY = squarePlayers[0].y + PLAYER_SIZE / 2;


    // Placera bollens målpositioner (fyra hörn)
    model->squareBallTargets[0][0] = squarePlayers[0].x + PLAYER_SIZE/2;
    model->squareBallTargets[0][1] = squarePlayers[0].y + PLAYER_SIZE/2;
    model->squareBallTargets[1][0] = squarePlayers[1].x + PLAYER_SIZE/2;
    model->squareBallTargets[1][1] = squarePlayers[1].y + PLAYER_SIZE/2;
    model->squareBallTargets[2][0] = squarePlayers[2].x + PLAYER_SIZE/2;
    model->squareBallTargets[2][1] = squarePlayers[2].y + PLAYER_SIZE/2;
    model->squareBallTargets[3][0] = squarePlayers[3].x + PLAYER_SIZE/2;
    model->squareBallTargets[3][1] = squarePlayers[3].y + PLAYER_SIZE/2;

    // Starta på första hörnet
    model->squareBallTargetIndex = 1;
    model->squareBallX = model->squareBallTargets[0][0];
    model->squareBallY = model->squareBallTargets[0][1];

    // Beräkna initial hastighet
    float deltaX = model->squareBallTargets[1][0] - model->squareBallX;
    float deltaY = model->squareBallTargets[1][1] - model->squareBallY;
    float length = sqrtf(deltaX * deltaX + deltaY * deltaY);
    if (length != 0) {
        model->squareBallVelX = (deltaX / length) * 3.0f;
        model->squareBallVelY = (deltaY / length) * 3.0f;
    }


}



void movePlayerTowards(Player *player, float targetX, float targetY, float speed, struct GameModel* model) {
    float deltaX = targetX - player->x;
    float deltaY = targetY - player->y;
    float distance = sqrt(deltaX * deltaX + deltaY * deltaY);

    if (distance > speed && distance != 0) {
        player->x += speed * (deltaX / distance);
        player->y += speed * (deltaY / distance);

        player->angle = atan2(deltaY, deltaX) * (180.0 / M_PI);
        if (player->angle < 0) player->angle += 360;

        player->animationState = RUN;
        Uint32 now = SDL_GetTicks();
        if (now - player->lastFrameTime > FRAME_DELAY) {
            int frameCount = animationFrameCounts[player->animationState];
            player->frame = (player->frame + 1) % frameCount;
            player->lastFrameTime = now;
        }
    } else {
        player->x = targetX;
        player->y = targetY;
        player->animationState = RUN;
        player->frame = 0;
        player->lastFrameTime = SDL_GetTicks();
    }
}


void updateSquareBall(GameModel* model) {
    model->squareBallX += model->squareBallVelX;
    model->squareBallY += model->squareBallVelY;

    // Kolla om bollen är tillräckligt nära målet
    float dx = model->squareBallTargets[model->squareBallTargetIndex][0] - model->squareBallX;
    float dy = model->squareBallTargets[model->squareBallTargetIndex][1] - model->squareBallY;
    if (fabsf(dx) < 5.0f && fabsf(dy) < 5.0f) {
        // Gå vidare till nästa hörn
        model->squareBallTargetIndex = (model->squareBallTargetIndex + 1) % 4;
        float targetX = model->squareBallTargets[model->squareBallTargetIndex][0];
        float targetY = model->squareBallTargets[model->squareBallTargetIndex][1];

        // Räkna ut ny hastighetsvektor
        float deltaX = targetX - model->squareBallX;
        float deltaY = targetY - model->squareBallY;
        float length = sqrtf(deltaX * deltaX + deltaY * deltaY);
        if (length != 0) {
            model->squareBallVelX = (deltaX / length) * 3.0f; // justera hastigheten här (px per frame)
            model->squareBallVelY = (deltaY / length) * 3.0f;
        }
    }
}

