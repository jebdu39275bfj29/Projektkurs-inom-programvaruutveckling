#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <SDL2/SDL.h>

// Forward-deklaration av GameModel (för att undvika cirkulära beroenden)
struct GameModel;

void updatePassingLogic(struct GameModel* model);

#endif
