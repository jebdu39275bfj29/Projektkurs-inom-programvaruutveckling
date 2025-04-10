#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <SDL2/SDL.h>

struct GameModel;

typedef struct GameTextures {
    SDL_Texture* playerTexture;
    SDL_Texture* grassTexture;
    SDL_Texture* coachTexture;
    SDL_Texture* ballTexture;
} GameTextures;

GameTextures loadAllTextures(SDL_Renderer* renderer);
void renderGame(SDL_Renderer* renderer, SDL_Texture* playerTexture, SDL_Texture* grassTexture, struct GameModel* model);

#endif
