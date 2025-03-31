#include "GameModel.h"

void initializeModel(struct GameModel* model) {
    // Placera spelarna i två trianglar
    model->players[0] = (Player){300, 200};
    model->players[1] = (Player){400, 100};
    model->players[2] = (Player){500, 200};

    model->players[3] = (Player){300, 400};
    model->players[4] = (Player){400, 500};
    model->players[5] = (Player){500, 400};

    // Startposition för tränaren
    model->coach = (Player){400, 300};

    // Passningssekvens
    int order[PLAYER_COUNT] = {0, 4, 1, 5, 2, 3};
    for (int i = 0; i < PLAYER_COUNT; i++) {
        model->passOrder[i] = order[i];
    }

    model->step = 0;
    model->lastPassTime = SDL_GetTicks();
}
