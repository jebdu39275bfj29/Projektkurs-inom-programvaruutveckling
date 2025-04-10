#include "GameView.h"
#include "GameModel.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdbool.h>

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
    textures.ballTexture   = loadTexture(renderer, "resources/Ball.png");
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

    const int frameHeights = 89;
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
        Player* c = &model->coach;
        int coachFrameW = animationFrameWidths[c->animationState];  
        int coachFrameH = frameHeights; 
        int coachRowIndex = 2; 

        SDL_Rect coachSrc = {
            model->coach.frame * coachFrameW,      
            coachRowIndex * coachFrameH,          
            coachFrameW,
            coachFrameH
        };

        SDL_Rect coachDst = {
            (int)(model->coach.x),
            (int)(model->coach.y),
            PLAYER_SIZE,
            PLAYER_SIZE
        };

        SDL_Point center = { PLAYER_SIZE / 2, PLAYER_SIZE / 2 };

        SDL_RendererFlip flip = SDL_FLIP_NONE;
        if (c->angle >= 90 && c->angle <= 270) {
            flip = SDL_FLIP_HORIZONTAL;
        }

        SDL_RenderCopyEx(renderer, model->coachTexture, &coachSrc, &coachDst, 0, &center, flip);

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

    // Rita boll
    if (model->ball.texture) {
        SDL_Rect dst;
        dst.x = (int)(model->ball.x);
        dst.y = (int)(model->ball.y);
        dst.w = model->ball.width;
        dst.h = model->ball.height;

        // Vi använder ball.angle för att rotera bollen för att simulera rullning
        // Här roterar vi runt mittpunkten av bollens sprite
        SDL_Point center = { dst.w / 2, dst.h / 2 };

        SDL_RenderCopyEx(renderer,
                         model->ball.texture,
                         NULL,
                         &dst,
                         model->ball.angle,
                         &center,
                         SDL_FLIP_NONE);
    }

    
    Player* carrier = &model->players[model->passOrder[model->step % PLAYER_COUNT]];

    float rad = carrier->angle * M_PI / 180.0f;
    float offsetX = cos(rad) * 20;
    float offsetY = sin(rad) * 20;
    
    bool isLeftSide = carrier->x < (WINDOW_WIDTH / 2);
    bool isMovingUp = carrier->angle > 180 && carrier->angle < 360;

    bool isRightSide = carrier->x > (WINDOW_WIDTH / 2);
    bool isMovingDown = carrier->angle > 60 && carrier->angle < 120;
    
    float additionalYOffset = 0;
    if (isLeftSide && isMovingUp) {
        additionalYOffset = 26; 
    }
    
    if (isRightSide && isMovingDown) {
        additionalYOffset = -26; 
    }

    model->ball.x = carrier->x + PLAYER_SIZE / 2 + offsetX - 16;
    model->ball.y = carrier->y + PLAYER_SIZE / 2 + offsetY + additionalYOffset;
    


    const int ballFrameSize = 72;
    const int ballRowIndex = 0;

    Uint32 now = SDL_GetTicks();
    if (now - model->ball.lastFrameTime > FRAME_DELAY) {
        model->ball.frame = (model->ball.frame + 1) % 7;
        model->ball.lastFrameTime = now;
    }

    SDL_Rect ballSrc = {
        model->ball.frame * ballFrameSize,
        ballRowIndex * ballFrameSize,
        ballFrameSize,
        ballFrameSize
    };

    SDL_Rect ballDst = {
        (int)model->ball.x,
        (int)model->ball.y,
        32,
        32
    };

        SDL_RenderCopy(renderer, model->ball.texture, &ballSrc, &ballDst);

        // --- NYTT: Rita cirkeln runt coachen ---
        int cx = (int)(model->coach.x + PLAYER_SIZE / 2);
        int cy = (int)(model->coach.y + PLAYER_SIZE / 2);
        int r = (int)model->coachDetectionRadius;
        
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Röd, lite transparens
        for (int angle = 0; angle < 360; angle += 2) {
            float rad = angle * M_PI / 180.0f;
            int x = cx + (int)(cosf(rad) * r);
            int y = cy + (int)(sinf(rad) * r);
            SDL_RenderDrawPoint(renderer, x, y);
        }
        
    

    
    SDL_RenderPresent(renderer);
}