#ifndef GAMEMODEL_H
#define GAMEMODEL_H

#include <SDL2/SDL.h>

#define PLAYER_COUNT 6
#define PLAYER_SIZE 50
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

typedef struct {
    float x, y;
} Player;

typedef struct GameModel {
    Player players[PLAYER_COUNT];  // Spelare i trianglar
    Player coach;                  // Tränarens position
    int passOrder[PLAYER_COUNT];   // Passningsordning
    int step;                      // Aktuell passningssteg
    Uint32 lastPassTime;          // Tid för senaste pass
} GameModel;

void initializeModel(struct GameModel* model);

#endif
