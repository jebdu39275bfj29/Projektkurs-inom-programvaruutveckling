#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

struct GameModel;
SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* path);
void renderGame(SDL_Renderer* renderer, SDL_Texture* playerTexture, SDL_Texture* grassTexture, struct GameModel* model);

#endif
