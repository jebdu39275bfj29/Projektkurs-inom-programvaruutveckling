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

    movePlayerTowards(&model->coach, coachTargetX, coachTargetY, MOVE_SPEED, model);

    if (fabs(model->players[from].x - targetX) < THRESHOLD &&
        fabs(model->players[from].y - targetY) < THRESHOLD) {
        model->step = (model->step + 1) % PLAYER_COUNT;
    }
}
