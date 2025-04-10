#include "GameView.h"
#include "GameModel.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

// Laddar textur
static SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        SDL_Log("IMG_Load Error: %s\n", IMG_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

GameTextures loadAllTextures(SDL_Renderer* renderer) {
    GameTextures textures = {0};
    textures.playerTexture = loadTexture(renderer, "resources/Players.png");
    textures.grassTexture = loadTexture(renderer, "resources/Pitch.png");
    textures.coachTexture = loadTexture(renderer, "resources/Coach.png");
    textures.ballTexture = loadTexture(renderer, "resources/Ball.png");
    return textures;
}

void renderGame(SDL_Renderer* renderer, SDL_Texture* playerTexture, SDL_Texture* grassTexture, struct GameModel* model) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Rita gräsplanen
    if (grassTexture) {
        SDL_Rect grassRect = { model->grass.x, model->grass.y, model->grass.width, model->grass.height };
        SDL_RenderCopy(renderer, grassTexture, NULL, &grassRect);
    }

    const int frameHeights = 87;
    const int spriteSheetRowMap[ANIMATION_COUNT] = { 0, 1, 2, 3, 4, 5 };
    const int animationFrameWidths[ANIMATION_COUNT] = { 101, 50, 40, 67, 50, 44 }; 

    // Rita spelarna med rotation
    for (int i = 0; i < PLAYER_COUNT; i++) {
        Player* p = &model->players[i];
        int rowIndex = spriteSheetRowMap[p->animationState];
        int frameW = animationFrameWidths[p->animationState];
        int frameH = frameHeights;

        SDL_Rect src = {
            p->frame * frameW,
            rowIndex * frameH,
            frameW,
            frameH
        };

        SDL_Rect dst = {
            (int)p->x,
            (int)p->y,
            PLAYER_SIZE,
            PLAYER_SIZE
        };

        SDL_Point center = { PLAYER_SIZE / 2, PLAYER_SIZE / 2 };

        SDL_RendererFlip flip = SDL_FLIP_NONE;
        if (p->angle >= 90 && p->angle <= 270) {
            flip = SDL_FLIP_HORIZONTAL;
        }

        SDL_RenderCopyEx(renderer, playerTexture, &src, &dst, 0, &center, flip);

    }

    // Rita coach-bilden
    if (model->coachTexture) {
        SDL_Rect coachRect = {
            (int)(model->coach.x - PLAYER_SIZE / 2),
            (int)(model->coach.y - PLAYER_SIZE / 2),
            PLAYER_SIZE,
            PLAYER_SIZE
        };
        SDL_RenderCopy(renderer, model->coachTexture, NULL, &coachRect);
    }

    // Rita passningslinje
    if (model->step > 0) {
        int from = model->passOrder[(model->step - 1) % PLAYER_COUNT];
        int to   = model->passOrder[model->step % PLAYER_COUNT];
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderDrawLine(renderer,
            (int)(model->players[from].x + PLAYER_SIZE / 2),
            (int)(model->players[from].y + PLAYER_SIZE / 2),
            (int)(model->players[to].x + PLAYER_SIZE / 2),
            (int)(model->players[to].y + PLAYER_SIZE / 2)
        );
    }

    // Rita bollen
    Player* carrier = &model->players[model->passOrder[model->step % PLAYER_COUNT]];
    float rad = model->ball.angle * M_PI / 180.0f;
    model->ball.x = carrier->x + cos(rad) * 20;
    model->ball.y = carrier->y + sin(rad) * 20;

    SDL_Rect ballSrc = { model->ball.frame * 64, 0, 64, 64 }; // välj rad 0 t.ex.
    SDL_Rect ballDst = { (int)model->ball.x, (int)model->ball.y, 32, 32 };
    SDL_RenderCopy(renderer, model->ball.texture, &ballSrc, &ballDst);

    SDL_RenderPresent(renderer);
}
