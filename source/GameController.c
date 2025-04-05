#include "GameController.h"
#include "GameModel.h"
#include "GameView.h"
#include <SDL2/SDL.h>
#include <math.h>
#include <stdio.h>

#define MOVE_SPEED 2.0f     
#define THRESHOLD 5.0f     
#define FRAME_DELAY 100
#define M_PI 3.14159265358979323846

const int animationFrameCounts[ANIMATION_COUNT] = {
    4,  
    8,  
    10, 
    6,  
    8,  
    9   
};

void movePlayerTowards(Player *player, float targetX, float targetY, float speed, struct GameModel* model) {
    float deltaX = targetX - player->x;
    float deltaY = targetY - player->y;
    float distance = sqrt(deltaX * deltaX + deltaY * deltaY);

    if (distance > speed && distance != 0) {
        player->x += speed * (deltaX / distance);
        player->y += speed * (deltaY / distance);
        
        player->angle = atan2(deltaY, deltaX) * (180.0 / M_PI);
        if (player->angle < 0) {
        player->angle += 360; // Gör vinkel positiv om den är negativ
    }

        
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


void updatePassingLogic(struct GameModel* model) {
    int from = model->passOrder[model->step % PLAYER_COUNT];
    int to   = model->passOrder[(model->step + 1) % PLAYER_COUNT];

    float targetX = model->players[to].x;
    float targetY = model->players[to].y;

    movePlayerTowards(&model->players[from], targetX, targetY, MOVE_SPEED, model);

    if (fabs(model->players[from].x - targetX) < THRESHOLD &&
        fabs(model->players[from].y - targetY) < THRESHOLD) {
        model->coach.x = targetX;
        model->coach.y = targetY;
        model->step = (model->step + 1) % PLAYER_COUNT;
    }
}

