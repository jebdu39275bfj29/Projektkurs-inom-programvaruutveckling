#include "GameView.h"
#include "GameModel.h" // Full definition krävs här

SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        SDL_Log("IMG_Load Error: %s\n", IMG_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void renderGame(SDL_Renderer* renderer, SDL_Texture* playerTexture, struct GameModel* model) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Rita spelare
    for (int i = 0; i < PLAYER_COUNT; i++) {
        SDL_Rect dst = {
            (int)model->players[i].x,
            (int)model->players[i].y,
            PLAYER_SIZE,
            PLAYER_SIZE
        };
        SDL_RenderCopy(renderer, playerTexture, NULL, &dst);
    }

    // Rita tränare som röd rektangel
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect coachRect = {
        (int)(model->coach.x + PLAYER_SIZE / 4),
        (int)(model->coach.y + PLAYER_SIZE / 4),
        PLAYER_SIZE / 2,
        PLAYER_SIZE / 2
    };
    SDL_RenderFillRect(renderer, &coachRect);

    // Rita passningslinje (grön)
    int from = model->passOrder[model->step % PLAYER_COUNT];
    int to   = model->passOrder[(model->step + 1) % PLAYER_COUNT];

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderDrawLine(
        renderer,
        (int)(model->players[from].x + PLAYER_SIZE / 2),
        (int)(model->players[from].y + PLAYER_SIZE / 2),
        (int)(model->players[to].x + PLAYER_SIZE / 2),
        (int)(model->players[to].y + PLAYER_SIZE / 2)
    );

    SDL_RenderPresent(renderer);
}
