#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <SDL2/SDL.h>
#include "GameModel.h" 
// Forward-deklaration av GameModel (för att undvika cirkulära beroenden)
struct GameModel;


int startGameLoop();
void updatePassingLogic(GameModel* model);
//void updateCoachPosition(float targetX, float targetY, GameModel *model);
void handle_pass(GameModel* model, int from, int to);
void update_ball(Ball* ball, Player players[], GameModel* model);
void update_players(Player players[]);
void updateTriangleLogic(GameModel* model);
void updateTriangleCoach(GameModel* model);
#endif
