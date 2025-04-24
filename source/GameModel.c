#include "GameModel.h"
#include <math.h>

const int animationFrameCounts[ANIMATION_COUNT] = {
    4, 8, 10, 6, 8, 9
};

void cleanupModel(struct GameModel* model) {
    if (model->coachTexture) {
        SDL_DestroyTexture(model->coachTexture);
        model->coachTexture = NULL;
    }
}


void initializeModel(struct GameModel* model, SDL_Texture* coachTexture)
 {
    model->grass.x = 10;
    model->grass.y = 10;
    model->grass.width = 780;
    model->grass.height = 580;
    model->grass.texture = NULL;
    model->currentPage = PAGE_MAIN;


    int offsetX = 10; 
    int offsetY = 10; 

    Uint32 startTime = SDL_GetTicks();

    model->players[0] = (Player){ model->grass.x, model->grass.y - offsetY, 0, startTime, RUN }; // vänster övre hörn
    model->players[1] = (Player){ model->grass.x + model->grass.width - (10 * offsetX), model->grass.y, 0, startTime, RUN }; // höger övre hörn
    model->players[2] = (Player){ model->grass.x - offsetX, model->grass.y + model->grass.height - (10 * offsetY), 0, startTime, RUN }; // vänster nedre hörn

    model->players[3] = (Player){ model->grass.x + model->grass.width - (10 * offsetX), model->grass.y + model->grass.height - (10 * offsetY), 0, startTime, RUN }; // höger nedre hörn
    model->players[4] = (Player){ model->grass.x + model->grass.width / 2, model->grass.y, 0, startTime, RUN }; // mitt övre
    model->players[5] = (Player){ model->grass.x + (model->grass.width + (2 * offsetX)) / 2, model->grass.y + model->grass.height - (11 * offsetY), 0, startTime, RUN }; // mitt nedre

    model->coach = (Player){
        //model->grass.x + model->grass.width / 2,
        //model->grass.y + model->grass.height / 2,
        model->grass.xCenter = model->grass.x + model->grass.width / 2,
        model->grass.yCenter = model->grass.y + model->grass.height / 2,
        0, 
        startTime, 
        IDLE
    };
    

    model->ball.x = model->coach.x;
    model->ball.y = model->coach.y;
    model->ball.frame = 0;
    model->ball.lastFrameTime = SDL_GetTicks();
    model->ball.angle = 0;
    model->ball.texture = NULL;

    model->coachTexture = coachTexture;
    model->coachDetectionRadius = 60.0f;

    model->coachManual = false;
    model->coachTargetX = model->coach.x;
    model->coachTargetY = model->coach.y;


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


    // Passordning: springaren -> punkt1 -> punkt2 -> punkt3
    model->trianglePassOrder[0] = 0; // vänster spets
    model->trianglePassOrder[1] = 1; // höger spets
    model->trianglePassOrder[2] = 2; // övre spets
    model->trianglePassOrder[3] = 3; // bottenmitten
    model->triangleStep = 0;



    int order[PLAYER_COUNT] = {4, 1, 3, 5, 2, 0}; 
    for (int i = 0; i < PLAYER_COUNT; i++) {
        model->passOrder[i] = order[i];
    }

    for (int i = 0; i < PLAYER_COUNT; i++) {
        model->players[i].originalX = model->players[i].x;
        model->players[i].originalY = model->players[i].y;
        model->players[i].targetX = model->players[i].x;
        model->players[i].targetY = model->players[i].y;
        model->players[i].hasBall = 0;
    }

    model->players[model->passOrder[0]].hasBall = 1;
    model->step = 0;
    model->lastPassTime = SDL_GetTicks();
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
