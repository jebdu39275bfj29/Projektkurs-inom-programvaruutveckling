#include "GameController.h"
#include "GameModel.h"  // Här behövs den kompletta definitionen

#define PASS_INTERVAL 1000 // ms

void updatePassingLogic(struct GameModel* model) {
    Uint32 now = SDL_GetTicks();

    if (now - model->lastPassTime >= PASS_INTERVAL) {
        model->step = (model->step + 1) % PLAYER_COUNT;
        model->lastPassTime = now;
    }

    int from = model->passOrder[model->step % PLAYER_COUNT];
    int to = model->passOrder[(model->step + 1) % PLAYER_COUNT];

    model->coach.x = (model->players[from].x + model->players[to].x) / 2;
    model->coach.y = (model->players[from].y + model->players[to].y) / 2;
}
