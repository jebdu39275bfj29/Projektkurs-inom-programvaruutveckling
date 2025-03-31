#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// Forward declare GameModel struct
struct GameModel;

SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* path);
void renderGame(SDL_Renderer* renderer, SDL_Texture* playerTexture, struct GameModel* model);

#endif
