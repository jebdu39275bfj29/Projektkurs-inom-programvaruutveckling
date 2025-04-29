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
    memset(model, 0, sizeof(GameModel));  // nollställer ALLT
    model->grass.x = 10;
    model->grass.y = 10;
    model->grass.width = 780;
    model->grass.height = 580;
    model->grass.texture = NULL;
    model->currentPage = PAGE_MAIN;

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
