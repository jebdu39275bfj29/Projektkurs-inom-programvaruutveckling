#ifndef GAMEMODEL_H
#define GAMEMODEL_H

#include <SDL2/SDL.h>

#define PLAYER_COUNT 6
#define PLAYER_SIZE 64
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

typedef enum {
    IDLE = 0,
    WALK,
    RUN,
    JUMP,
    SLIDE,
    SHOOT,
    ANIMATION_COUNT
} AnimationState;

typedef struct {
    float x, y;
    int frame;
    Uint32 lastFrameTime;
    int animationState;
    float angle;
} Player;

typedef struct {
    int x, y;
    int width, height;
    SDL_Texture *texture;
} Grass;

typedef struct GameModel {
    Player players[PLAYER_COUNT];  // Spelare
    Player coach;                  // Bollen (eller coachen)
    SDL_Texture* coachTexture;
    int passOrder[PLAYER_COUNT];   // Passningsordning
    int step;                      // Aktuellt passningssteg
    Uint32 lastPassTime;           // Tid för senaste pass
    Grass grass;
} GameModel;

void cleanupModel(struct GameModel* model);
void initializeModel(struct GameModel* model, SDL_Renderer* renderer);

#endif
