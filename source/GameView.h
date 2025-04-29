#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <SDL2/SDL.h>
#include <stdbool.h>

struct GameModel;

typedef struct GameTextures {
    SDL_Texture* playerTexture;
    SDL_Texture* grassTexture;
    SDL_Texture* coachTexture;
    SDL_Texture* ballTexture;
    SDL_Texture* triangleTexture;
} GameTextures;

GameTextures loadAllTextures(SDL_Renderer* renderer);
void renderGame(SDL_Renderer* renderer, SDL_Texture* playerTexture, SDL_Texture* grassTexture, struct GameModel* model);
bool initializeTextSystem(void);
void renderTriangleScene(SDL_Renderer* renderer, struct GameModel* model, SDL_Texture* playerTexture, SDL_Texture* background);


#endif
