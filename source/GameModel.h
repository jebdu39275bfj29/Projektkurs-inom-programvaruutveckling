#ifndef GAMEMODEL_H
#define GAMEMODEL_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <math.h> 


#define PLAYER_COUNT 6
#define PLAYER_SIZE 64
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define FRAME_DELAY 100
#define BALL_SPEED 5.0f
#define PLAYER_SPEED 1.5f
#define MOVE_SPEED 1.5f     
#define THRESHOLD 5.0f  
#define MAX_PLAYERS PLAYER_COUNT 
#define COACH_SPEED 1.5f
#define M_PI 3.14159265358979323846
#define PLAYER_COUNT_TRIANGLE 4

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

typedef enum {
    ATTACHED,
    IN_FLIGHT
} BallState;

typedef struct {
    float x, y;               // Position
    float angle;              // Rotation
    int width, height;        // Storlek i pixlar
    int frame;                // Animationsframe
    Uint32 lastFrameTime;     // Tid för senaste frame
    SDL_Texture *texture;     // Textur
    BallState state;          // Tillstånd för bollen
    int currentPlayerIndex;   // Om bollen är fäst vid spelare
    float targetX, targetY;   // Målposition om bollen flyger

    int attachedPlayer;       // Spelaren bollen är fäst vid
    float velX;               // Hastighet X
    float velY;               // Hastighet Y
} Ball;

typedef struct {
    float x, y;
    int frame;
    Uint32 lastFrameTime;
    int animationState;
    float angle;

    int state;            
    float targetX;        
    float targetY;  
    float originalX;
    float originalY;     
    float startX;
    float startY;
    int hasBall;
} Player;

typedef struct {
    int x, y;
    int width, height;
    int xCenter, yCenter;
    SDL_Texture *texture;
} Grass;

typedef enum {
    PAGE_MAIN,
    PAGE_EMPTY
} PageState;

typedef struct GameModel {
    Player players[PLAYER_COUNT];
    Player coach;
    SDL_Texture* coachTexture;
    int coachAnimationState;
    int coachFrame;
    Uint32 coachLastFrameTime;
    int passOrder[PLAYER_COUNT];
    float coachDetectionRadius;
    int step;
    Uint32 lastPassTime;
    Grass grass;
    Ball ball;
    int activePlayer;

    bool passCompleted;
    bool passInitiated;
    float emptyX, emptyY;

    bool  coachManual;     
    float coachTargetX;    
    float coachTargetY;

    PageState currentPage;

    Player trianglePlayers[PLAYER_COUNT_TRIANGLE];
    int trianglePassOrder[PLAYER_COUNT_TRIANGLE];
    int triangleStep;
    bool trianglePassInitiated;
    bool trianglePassCompleted;
} GameModel;

void cleanupModel(struct GameModel* model);
void initializeModel(struct GameModel* model, SDL_Texture* coachTexture);
void movePlayerTowards(Player *player, float targetX, float targetY, float speed, struct GameModel* model);

#endif
