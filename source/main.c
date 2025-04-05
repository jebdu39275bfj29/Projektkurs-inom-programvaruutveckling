#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include "GameModel.h"
#include "GameView.h"
#include "GameController.h"

int main(int argc, char* argv[]) {
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
    
    SDL_Texture* playerTexture = loadTexture(renderer, "resources/Players.png");
    if (!playerTexture) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Texture* grassTexture = loadTexture(renderer, "resources/Pitch.png");
    if (!grassTexture) {
        SDL_DestroyTexture(playerTexture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    GameModel model;
    initializeModel(&model, renderer);

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // Uppdatera dribbling och passning
        updatePassingLogic(&model);
        renderGame(renderer, playerTexture, grassTexture, &model);

        SDL_Delay(16); // ~60 FPS
    }

    // Rensa resurser innan avslutning
    cleanupModel(&model);
    SDL_DestroyTexture(playerTexture);
    SDL_DestroyTexture(grassTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
