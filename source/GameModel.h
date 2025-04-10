// --- GameModel.h ---
#ifndef GAMEMODEL_H
#define GAMEMODEL_H

#include <SDL2/SDL.h>

#define PLAYER_COUNT 6
#define PLAYER_SIZE 64
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define FRAME_DELAY 100

typedef enum {
    IDLE = 0,
    WALK,
    RUN,
    JUMP,
    SLIDE,
    SHOOT,
    ANIMATION_COUNT
} AnimationState;

extern const int animationFrameCounts[ANIMATION_COUNT];

typedef struct {
    float x, y;         // Bollens position
    float angle;        // Hur mycket bollen har roterat (för att simulera rullning)
    int width, height;  // Bollens storlek
    SDL_Texture *texture;
} Ball;

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
    Player players[PLAYER_COUNT];
    Player coach;
    SDL_Texture* coachTexture;
    int passOrder[PLAYER_COUNT];
    int step;
    Uint32 lastPassTime;
    Grass grass;
    Ball ball;
} GameModel;

void cleanupModel(struct GameModel* model);
void initializeModel(struct GameModel* model, SDL_Texture* coachTexture);
void movePlayerTowards(Player *player, float targetX, float targetY, float speed, struct GameModel* model);

#endif