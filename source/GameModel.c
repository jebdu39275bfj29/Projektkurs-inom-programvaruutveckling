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
    model->grass.x = 150;
    model->grass.y = 150;
    model->grass.width = 500;
    model->grass.height = 300;
    model->grass.texture = NULL;

    int offsetX = 60; 
    int offsetY = 60; 

    Uint32 startTime = SDL_GetTicks();

    model->players[0] = (Player){ model->grass.x - offsetX, model->grass.y, 0, startTime, IDLE };
    model->players[1] = (Player){ model->grass.x + model->grass.width + offsetX, model->grass.y, 0, startTime, IDLE };
    model->players[2] = (Player){ model->grass.x - offsetX, model->grass.y + model->grass.height, 0, startTime, IDLE };
    model->players[3] = (Player){ model->grass.x + model->grass.width + offsetX, model->grass.y + model->grass.height, 0, startTime, IDLE };
    model->players[4] = (Player){ model->grass.x + model->grass.width / 2, model->grass.y - offsetY, 0, startTime, IDLE };
    model->players[5] = (Player){ model->grass.x + model->grass.width / 2, model->grass.y + model->grass.height + offsetY, 0, startTime, IDLE };

    model->coach = (Player){
        model->grass.x + model->grass.width / 2,
        model->grass.y + model->grass.height / 2,
        0, startTime, IDLE
    };

    model->coachTexture = coachTexture;

    int order[PLAYER_COUNT] = {4, 1, 3, 5, 2, 0}; 
    for (int i = 0; i < PLAYER_COUNT; i++) {
        model->passOrder[i] = order[i];
    }

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
        player->animationState = IDLE;
        player->frame = 0;
    }
}