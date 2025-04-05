#include "GameModel.h"
#include <SDL2/SDL.h>
#include "GameView.h"

void cleanupModel(struct GameModel* model) {
    if (model->coachTexture) {
        SDL_DestroyTexture(model->coachTexture);
        model->coachTexture = NULL;
    }
}

void initializeModel(struct GameModel* model, SDL_Renderer* renderer) {
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

    model->coachTexture = loadTexture(renderer, "resources/Coach.png");

    int order[PLAYER_COUNT] = {4, 1, 3, 5, 2, 0}; 
    for (int i = 0; i < PLAYER_COUNT; i++) {
        model->passOrder[i] = order[i];
    }

    model->step = 0;
    model->lastPassTime = SDL_GetTicks();
}
