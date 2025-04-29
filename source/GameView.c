#include "GameView.h"
#include "GameModel.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL_ttf.h>

// Variabel för font
static TTF_Font* font = NULL;

bool initializeTextSystem() {
    if (TTF_Init() < 0) {
        SDL_Log("TTF_Init Error: %s", TTF_GetError());
        return false;
    }

    font = TTF_OpenFont("resources/Arial.ttf", 16);
    if (!font) {
        SDL_Log("Failed to load font: %s", TTF_GetError());
        return false;
    }
    return true;
}

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
    textures.triangleTexture = loadTexture(renderer, "resources/Triangle.png");
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
    const int animationFrameWidths[ANIMATION_COUNT] = { 48, 50, 40, 67, 50, 44}; 

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

    //Rita CIRKELN runt coachen (Hörselavstånd)
    // Aktivera blandningsläge för alfakanaler (genomskinlighet)
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Ljusare grön nyans och mer genomskinlighet
    SDL_SetRenderDrawColor(renderer, 144, 238, 144, 60);

    // Mittpunkt för cirkeln (coachens centrum)
    int circleCx = (int)(model->coach.x + PLAYER_SIZE / 2);
    int circleCy = (int)(model->coach.y + PLAYER_SIZE / 2);
    int r = (int)model->coachDetectionRadius;

    // Fyll cirkeln via pixel-loop
    for (int dy = -r; dy <= r; dy++) {
        int dxMax = (int)sqrtf((float)(r*r - dy*dy));
        for (int dx = -dxMax; dx <= dxMax; dx++) {
            SDL_RenderDrawPoint(renderer, circleCx + dx, circleCy + dy);
        }
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
    /*if (model->ball.texture) {
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
    }*/

    
    /*Player* carrier = &model->players[model->passOrder[model->step % PLAYER_COUNT]];

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

    SDL_RenderCopy(renderer, model->ball.texture, &ballSrc, &ballDst);*/

    const int ballFrameSize = 72;
    const int ballRowIndex = 0;
    Uint32 now = SDL_GetTicks();

    for (int bi = 0; bi < 2; bi++) {
        Ball* ball = &model->balls[bi];
        Player* carrier = &model->players[ball->attachedPlayer];

        float rad = carrier->angle * M_PI / 180.0f;
        float offsetX = cosf(rad) * 20.0f;
        float offsetY = sinf(rad) * 20.0f;

        bool isLeftSide   = carrier->x < (WINDOW_WIDTH / 2);
        bool isMovingUp   = carrier->angle > 180 && carrier->angle < 360;
        bool isRightSide  = carrier->x > (WINDOW_WIDTH / 2);
        bool isMovingDown = carrier->angle > 60  && carrier->angle < 120;

        float additionalYOffset = 0.0f;
        if (isLeftSide  && isMovingUp)   additionalYOffset =  26.0f;
        if (isRightSide && isMovingDown) additionalYOffset = -26.0f;

        ball->x = carrier->x + PLAYER_SIZE/2 + offsetX - 16;
        ball->y = carrier->y + PLAYER_SIZE/2 + offsetY + additionalYOffset;

        if (now - ball->lastFrameTime > FRAME_DELAY) {
            ball->frame = (ball->frame + 1) % 7;
            ball->lastFrameTime = now;
        }

        SDL_Rect src = {
            ball->frame * ballFrameSize,
            ballRowIndex * ballFrameSize,
            ballFrameSize,
            ballFrameSize
        };
        SDL_Rect dst = {
            (int)ball->x,
            (int)ball->y,
            32, 32
        };
        SDL_RenderCopy(renderer, ball->texture, &src, &dst);
    }

    //Coachens syn‑sektor
    const float  fovTotalRad  = 100.0f * M_PI / 180.0f;   //100°
    const float  fovHalfRad   = 0.5f * fovTotalRad;
    const float  visionLength = model->coachDetectionRadius * 5.0f; //25 m

    /* Coachens mittpunkt + riktningsvinkel */
    float forwardRad = model->coach.angle * M_PI / 180.0f;
    int   apexX      = (int)(model->coach.x + PLAYER_SIZE / 2);
    int   apexY      = (int)(model->coach.y + PLAYER_SIZE / 2);

    /* Aktivera alfa‑blending */
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    //Sektor: radiellt utåt i 1‑px steg               
    int   stepsRad = (int)visionLength;
    float maxAlpha = 60.0f;                 
    for (int rStep = 0; rStep < stepsRad; ++rStep) {

        float  currLen = (float)rStep;
        float  nextLen = currLen + 1.0f;

        /* Kvadratisk fade: (1 - (r/R)^2) */
        float  t     = currLen / visionLength;          // 0 … 1
        float  fade  = sqrtf( t );                      // svagt nära, starkare ute
        Uint8  alpha   = (Uint8)(maxAlpha * fade);

        // Mjuk gul‑grön
        SDL_SetRenderDrawColor(renderer, 220, 230, 170, alpha);

        for (float a = -fovHalfRad; a <= fovHalfRad; a += (M_PI / 180.0f)) {

            float ca = cosf(forwardRad + a);
            float sa = sinf(forwardRad + a);

            int x1 = (int)(apexX + ca * currLen);
            int y1 = (int)(apexY + sa * currLen);
            int x2 = (int)(apexX + ca * nextLen);
            int y2 = (int)(apexY + sa * nextLen);

            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        }
    }

        // Coachens mittpunkt
        float coachCenterX = model->coach.x + PLAYER_SIZE / 2.0f;
        float coachCenterY = model->coach.y + PLAYER_SIZE / 2.0f;

        // 100 pixlar = 5 meter => 1 px = 0.05 m
        float pxToMeter = 5.0f / model->coachDetectionRadius;

        // Här lägger du texten rad för rad
        for (int i = 0; i < PLAYER_COUNT; i++) {
            // Spelarens mittpunkt
            float playerCenterX = model->players[i].x + PLAYER_SIZE / 2.0f;
            float playerCenterY = model->players[i].y + PLAYER_SIZE / 2.0f;

            float dx = playerCenterX - coachCenterX;
            float dy = playerCenterY - coachCenterY;
            float distPixels = sqrtf(dx*dx + dy*dy);

            // Omvandla pixelavstånd -> meteravstånd
            float distMeters = distPixels * pxToMeter;

            // Bygg textsträng
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "P%d: %.2f m", i, distMeters);

            // Skapa surface + textur av 'buffer' (SDL_ttf)
            SDL_Color white = {255, 255, 255, 255};
            SDL_Surface* textSurface = TTF_RenderText_Blended(font, buffer, white);
            if (textSurface) {
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                if (textTexture) {
                    // Bestäm en position för texten
                    SDL_Rect textRect;
                    textRect.x = 10;              // 10 pixlar från vänster
                    textRect.y = 10 + i * 20;     // 10 pixlar från toppen + 20 per spelare
                    textRect.w = textSurface->w;
                    textRect.h = textSurface->h;

                    // Rita texten
                    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

                    SDL_DestroyTexture(textTexture);
                }
                SDL_FreeSurface(textSurface);
            }
        }


    // Fyll kvadrat med rosa färg
    SDL_SetRenderDrawColor(renderer, 255, 105, 180, 255); // rosa (Hot Pink)
    SDL_Rect squareButton = {WINDOW_WIDTH - 180, 10, 30, 20};
    SDL_RenderFillRect(renderer, &squareButton);

    // Vit kantlinje runt kvadraten
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &squareButton);


    // Fyll triangel med röd färg
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // röd
    SDL_Point triangle[3] = {
        {WINDOW_WIDTH - 60, 10},
        {WINDOW_WIDTH - 30, 10},
        {WINDOW_WIDTH - 45, 30}
    };
    for (int y = 10; y <= 30; ++y) {
        for (int x = WINDOW_WIDTH - 60; x <= WINDOW_WIDTH - 30; ++x) {
            // enkel punkt-i-triangel-test (Barycentrisk metod vore bättre, men detta räcker!)
            int dx1 = triangle[1].x - triangle[0].x, dy1 = triangle[1].y - triangle[0].y;
            int dx2 = triangle[2].x - triangle[0].x, dy2 = triangle[2].y - triangle[0].y;
            int dx = x - triangle[0].x, dy = y - triangle[0].y;

            float s = (float)(dx * dy2 - dy * dx2) / (dx1 * dy2 - dy1 * dx2);
            float t = (float)(dx * dy1 - dy * dx1) / (dx2 * dy1 - dy2 * dx1);

            if (s >= 0 && t >= 0 && (s + t) <= 1) {
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }

    // Fyll rektangel med blå färg
    SDL_Rect backButton = {WINDOW_WIDTH - 120, 10, 40, 20};
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // blå
    SDL_RenderFillRect(renderer, &backButton);

    // Vit kantlinje runt triangel och rektangel
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); 
    SDL_Point triangleOutline[4] = {
        {WINDOW_WIDTH - 60, 10},
        {WINDOW_WIDTH - 30, 10},
        {WINDOW_WIDTH - 45, 30},
        {WINDOW_WIDTH - 60, 10}
    };
    SDL_RenderDrawLines(renderer, triangleOutline, 4);
    SDL_RenderDrawRect(renderer, &backButton);
    
    SDL_RenderPresent(renderer);
}


void renderTriangleScene(SDL_Renderer* renderer, GameModel* model, SDL_Texture* playerTexture, SDL_Texture* background) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Rect dst = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_RenderCopy(renderer, background, NULL, &dst);

    // Rita knappar
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // röd triangel
    for (int y = 10; y <= 30; ++y) {
        for (int x = WINDOW_WIDTH - 60; x <= WINDOW_WIDTH - 30; ++x) {
            int dx1 = -15, dy1 = 20;
            int dx2 = 15, dy2 = 20;
            int dx = x - (WINDOW_WIDTH - 45);
            int dy = y - 10;

            float s = (float)(dx * dy2 - dy * dx2) / (dx1 * dy2 - dy1 * dx2);
            float t = (float)(dx * dy1 - dy * dx1) / (dx2 * dy1 - dy2 * dx1);

            if (s >= 0 && t >= 0 && (s + t) <= 1) {
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // blå rektangel
    SDL_Rect backButton = {WINDOW_WIDTH - 120, 10, 40, 20};
    SDL_RenderFillRect(renderer, &backButton);

    // Vit kantlinje
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Point triangleOutline[4] = {
        {WINDOW_WIDTH - 45, 10},
        {WINDOW_WIDTH - 60, 30},
        {WINDOW_WIDTH - 30, 30},
        {WINDOW_WIDTH - 45, 10}
    };
    SDL_RenderDrawLines(renderer, triangleOutline, 4);
    SDL_RenderDrawRect(renderer, &backButton);

      // Fyll kvadrat med rosa färg
    SDL_SetRenderDrawColor(renderer, 255, 105, 180, 255); // rosa (Hot Pink)
    SDL_Rect squareButton = {WINDOW_WIDTH - 180, 10, 30, 20};
    SDL_RenderFillRect(renderer, &squareButton);

    // Vit kantlinje runt kvadraten
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &squareButton);


    // Rita de fyra triangel-spelarna
    const int frameHeights = 89;
    const int animationFrameWidths[ANIMATION_COUNT] = { 48, 50, 40, 67, 50, 44 };
    const int spriteSheetRowMap[ANIMATION_COUNT] = { 0, 1, 2, 3, 4, 5 };

    for (int i = 0; i < 4; i++) {
        Player* p = &model->trianglePlayers[i];
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


    // Rita boll som följer den springande spelaren
    for (int i = 0; i < 4; ++i) {
        Player* p = &model->trianglePlayers[i];
        if (p->hasBall) {
            float rad = p->angle * M_PI / 180.0f;
            float offsetX = cos(rad) * 20;
            float offsetY = sin(rad) * 20;

            model->ball.x = p->x + PLAYER_SIZE / 2 + offsetX - 16;
            model->ball.y = p->y + PLAYER_SIZE / 2 + offsetY;

            Uint32 now = SDL_GetTicks();
            if (now - model->ball.lastFrameTime > FRAME_DELAY) {
                model->ball.frame = (model->ball.frame + 1) % 7;
                model->ball.lastFrameTime = now;
            }

            SDL_Rect ballSrc = {
                model->ball.frame * 72,
                0,
                72,
                72
            };

            SDL_Rect ballDst = {
                (int)model->ball.x,
                (int)model->ball.y,
                32,
                32
            };

            SDL_RenderCopy(renderer, model->ball.texture, &ballSrc, &ballDst);
            break;  
        }
    }

    // TRIANGELCOACHENS HÖRSELCIRKEL
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 144, 238, 144, 60);

    int circleCx = (int)(model->triangleCoach.x + PLAYER_SIZE / 2);
    int circleCy = (int)(model->triangleCoach.y + PLAYER_SIZE / 2);
    int r = (int)model->coachDetectionRadius;

    for (int dy = -r; dy <= r; dy++) {
        int dxMax = (int)sqrtf((float)(r*r - dy*dy));
        for (int dx = -dxMax; dx <= dxMax; dx++) {
            SDL_RenderDrawPoint(renderer, circleCx + dx, circleCy + dy);
        }
    }

    // TRIANGELCOACHENS SYN-SEKTOR
    const float  fovTotalRad  = 100.0f * M_PI / 180.0f;
    const float  fovHalfRad   = 0.5f * fovTotalRad;
    const float  visionLength = model->coachDetectionRadius * 3.0f; //15m

    float forwardRad = model->triangleCoach.angle * M_PI / 180.0f;
    int   apexX      = (int)(model->triangleCoach.x + PLAYER_SIZE / 2);
    int   apexY      = (int)(model->triangleCoach.y + PLAYER_SIZE / 2);

    float maxAlpha = 60.0f;
    for (int rStep = 0; rStep < (int)visionLength; ++rStep) {
        float currLen = (float)rStep;
        float nextLen = currLen + 1.0f;

        float t = currLen / visionLength;
        float fade = sqrtf(t);
        Uint8 alpha = (Uint8)(maxAlpha * fade);

        SDL_SetRenderDrawColor(renderer, 220, 230, 170, alpha);

        for (float a = -fovHalfRad; a <= fovHalfRad; a += (M_PI / 180.0f)) {
            float ca = cosf(forwardRad + a);
            float sa = sinf(forwardRad + a);

            int x1 = (int)(apexX + ca * currLen);
            int y1 = (int)(apexY + sa * currLen);
            int x2 = (int)(apexX + ca * nextLen);
            int y2 = (int)(apexY + sa * nextLen);

            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        }
    }

    if (playerTexture) {
        const int frameHeights = 89;
        const int animationFrameWidths[ANIMATION_COUNT] = { 48, 50, 40, 67, 50, 44 };
        //const int spriteSheetRowMap[ANIMATION_COUNT] = { 0, 1, 2, 3, 4, 5 };

        int rowIndex = 2;
        int frameW = animationFrameWidths[model->triangleCoach.animationState];
        int frameH = frameHeights;

        SDL_Rect src = {
            model->triangleCoach.frame * frameW,
            rowIndex * frameH,
            frameW,
            frameH
        };

        SDL_Rect dst = {
            (int)model->triangleCoach.x,
            (int)model->triangleCoach.y,
            PLAYER_SIZE,
            PLAYER_SIZE
        };

        SDL_Point center = { PLAYER_SIZE / 2, PLAYER_SIZE / 2 };

        Uint32 now = SDL_GetTicks();
        if (model->triangleCoach.animationState != IDLE) {
            Uint32 now = SDL_GetTicks();
            if (now - model->triangleCoach.lastFrameTime > FRAME_DELAY) {
                int frameCount = animationFrameCounts[model->triangleCoach.animationState];
                model->triangleCoach.frame = (model->triangleCoach.frame + 1) % frameCount;
                model->triangleCoach.lastFrameTime = now;
            }
        }

        SDL_RendererFlip flip = SDL_FLIP_NONE;
        if (model->triangleCoach.angle >= 90 && model->triangleCoach.angle <= 270) {
            flip = SDL_FLIP_HORIZONTAL;
        }
        SDL_RenderCopyEx(renderer, model->coachTexture, &src, &dst, 0.0, &center, flip);

    }

    // TRIANGELCOACHENS AVSTÅNDSPARAMETRAR
    float triangleCoachCenterX = model->triangleCoach.x + PLAYER_SIZE / 2.0f;
    float triangleCoachCenterY = model->triangleCoach.y + PLAYER_SIZE / 2.0f;

    float pxToMeter = 5.0f / model->coachDetectionRadius;

    for (int i = 0; i < 4; i++) {
        float playerCenterX = model->trianglePlayers[i].x + PLAYER_SIZE / 2.0f;
        float playerCenterY = model->trianglePlayers[i].y + PLAYER_SIZE / 2.0f;

        float dx = playerCenterX - triangleCoachCenterX;
        float dy = playerCenterY - triangleCoachCenterY;
        float distPixels = sqrtf(dx * dx + dy * dy);
        float distMeters = distPixels * pxToMeter;

        char buffer[64];
        snprintf(buffer, sizeof(buffer), "T%d: %.2f m", i, distMeters);

        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface* textSurface = TTF_RenderText_Blended(font, buffer, white);
        if (textSurface) {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            if (textTexture) {
                SDL_Rect textRect;
                textRect.x = 10;
                textRect.y = 10 + i * 20;
                textRect.w = textSurface->w;
                textRect.h = textSurface->h;

                SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                SDL_DestroyTexture(textTexture);
            }
            SDL_FreeSurface(textSurface);
        }
    }


    SDL_RenderPresent(renderer);
}


void renderSquareScene(SDL_Renderer* renderer, struct GameModel* model, SDL_Texture* grassTexture, SDL_Texture* playerTexture)
 {
    // Bakgrundsfärg (mörkgrå)
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    // Rita gräsplanen (samma som i renderGame)
    if (grassTexture) {
        SDL_Rect grassRect = {
            model->grass.x,
            model->grass.y,
            model->grass.width,
            model->grass.height
        };
        SDL_RenderCopy(renderer, grassTexture, NULL, &grassRect);
    }

    // 🎯 RITA TILLBAKA-KNAPP (blå rektangel)
    SDL_Rect backButton = {WINDOW_WIDTH - 120, 10, 40, 20};
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // blå
    SDL_RenderFillRect(renderer, &backButton);

    // Fyll triangel med röd färg
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // röd
    SDL_Point triangle[3] = {
        {WINDOW_WIDTH - 60, 10},
        {WINDOW_WIDTH - 30, 10},
        {WINDOW_WIDTH - 45, 30}
    };
    for (int y = 10; y <= 30; ++y) {
        for (int x = WINDOW_WIDTH - 60; x <= WINDOW_WIDTH - 30; ++x) {
            // enkel punkt-i-triangel-test (Barycentrisk metod vore bättre, men detta räcker!)
            int dx1 = triangle[1].x - triangle[0].x, dy1 = triangle[1].y - triangle[0].y;
            int dx2 = triangle[2].x - triangle[0].x, dy2 = triangle[2].y - triangle[0].y;
            int dx = x - triangle[0].x, dy = y - triangle[0].y;

            float s = (float)(dx * dy2 - dy * dx2) / (dx1 * dy2 - dy1 * dx2);
            float t = (float)(dx * dy1 - dy * dx1) / (dx2 * dy1 - dy2 * dx1);

            if (s >= 0 && t >= 0 && (s + t) <= 1) {
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }


    // RITA KVADRATKNAPP (rosa)
    SDL_Rect squareButton = {WINDOW_WIDTH - 180, 10, 30, 20};
    SDL_SetRenderDrawColor(renderer, 255, 105, 180, 255); // rosa (Hot Pink)
    SDL_RenderFillRect(renderer, &squareButton);

    // 🧾 VITA KANTLINJER
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    // Triangelkontur
    SDL_Point triangleOutline[4] = {
        {WINDOW_WIDTH - 60, 10},
        {WINDOW_WIDTH - 30, 10},
        {WINDOW_WIDTH - 45, 30},
        {WINDOW_WIDTH - 60, 10}
    };
    SDL_RenderDrawLines(renderer, triangleOutline, 4);

    // Rektangelkontur
    SDL_RenderDrawRect(renderer, &backButton);

    // Kvadratkontur
    SDL_RenderDrawRect(renderer, &squareButton);


    const int frameHeights = 89;
    const int animationFrameWidths[ANIMATION_COUNT] = { 48, 50, 40, 67, 50, 44 };
    const int spriteSheetRowMap[ANIMATION_COUNT] = { 0, 1, 2, 3, 4, 5 };

    for (int i = 0; i < SQUARE_PLAYER_COUNT; ++i) {
        Player* p = &squarePlayers[i];
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


    const int ballFrameSize = 72;
    const int ballRowIndex = 0; 
    Uint32 now = SDL_GetTicks();

    // Uppdatera animationen (byter frame)
    if (now - model->squareBall.lastFrameTime > FRAME_DELAY) {
        model->squareBall.frame = (model->squareBall.frame + 1) % 7;
        model->squareBall.lastFrameTime = now;
    }


    // Rita rätt frame från spritesheeten
    SDL_Rect src = {
        model->squareBall.frame * ballFrameSize,
        ballRowIndex * ballFrameSize,
        ballFrameSize,
        ballFrameSize
    };

    SDL_Rect dst = {
        (int)model->squareBallX,
        (int)model->squareBallY,
        32, 32
    };

    SDL_RenderCopy(renderer, model->squareBall.texture, &src, &dst);

    SDL_RenderPresent(renderer);
}

