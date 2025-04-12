#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <SDL2/SDL.h>

// Forward-deklaration av GameModel (för att undvika cirkulära beroenden)
struct GameModel;

int startGameLoop();

void updatePassingLogic(struct GameModel* model);
void steerCoachNoStuck(struct GameModel* model, float targetX, float targetY, float speed);


#endif
