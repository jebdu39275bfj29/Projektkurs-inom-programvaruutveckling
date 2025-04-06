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

    movePlayerTowards(&model->players[from], targetX, targetY, MOVE_SPEED, model);

    if (fabs(model->players[from].x - targetX) < THRESHOLD &&
        fabs(model->players[from].y - targetY) < THRESHOLD) {
        model->coach.x = targetX;
        model->coach.y = targetY;
        model->step = (model->step + 1) % PLAYER_COUNT;
    }
}

