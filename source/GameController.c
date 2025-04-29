#include "GameController.h"
#include "GameModel.h"
#include "GameView.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdbool.h>

static void coachIdle(Player* coach){
    if (coach->animationState != IDLE) {
        coach->animationState = IDLE;
        coach->frame          = 0;          // första bilden i idle‑raden
        coach->lastFrameTime  = SDL_GetTicks();
    }
}


static bool handleEvents(GameModel* model)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {

        if (event.type == SDL_QUIT)
            return false;

        if (event.type == SDL_MOUSEBUTTONDOWN &&
            event.button.button == SDL_BUTTON_LEFT) {

            int mx = event.button.x;
            int my = event.button.y;

            bool clickedButton = false;

            // Triangelknapp: gå till tom sida
            if (mx >= WINDOW_WIDTH - 60 && mx <= WINDOW_WIDTH - 30 && my >= 10 && my <= 30) {
                if (model->currentPage != PAGE_EMPTY) {
                    model->currentPage = PAGE_EMPTY;
                }
                clickedButton = true;
            }

            // Rektangelknapp: gå tillbaka
            else if (mx >= WINDOW_WIDTH - 120 && mx <= WINDOW_WIDTH - 80 && my >= 10 && my <= 30) {
                if (model->currentPage != PAGE_MAIN) {
                    model->currentPage = PAGE_MAIN;
                }
                clickedButton = true;
            }

            // Endast trigga coach om man INTE tryckt på knapp
            if (!clickedButton && model->currentPage == PAGE_MAIN) {
                model->coachTargetX = (float)mx - PLAYER_SIZE / 2.0f;
                model->coachTargetY = (float)my - PLAYER_SIZE / 2.0f;
                model->coachManual = true;
            }

            // Triangel-coachen
            if (!clickedButton && model->currentPage == PAGE_EMPTY) {
                model->triangleCoachManual = true;
                model->triangleCoachTargetX = (float)mx - PLAYER_SIZE / 2.0f;
                model->triangleCoachTargetY = (float)my - PLAYER_SIZE / 2.0f;
            }

        }
    }
    return true;
}


int startGameLoop() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        SDL_Log("IMG_Init Error: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }

    if (!initializeTextSystem()) {
        SDL_Log("TTF Error:\n");
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Football Simulation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    GameTextures textures = loadAllTextures(renderer);

    if (!textures.playerTexture || !textures.grassTexture || !textures.coachTexture) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    GameModel model;
    initializeModel(&model, textures.coachTexture);
    model.balls[0].texture = textures.ballTexture;
    model.balls[1].texture = textures.ballTexture;
    model.activePlayer = model.passOrder[0];
    //model.passInitiated = true;
    //model.passCompleted = true;
    //handle_pass(&model, model.passOrder[0], model.passOrder[1]);
    model.balls[0].attachedPlayer = model.passOrder[0];
    model.balls[1].attachedPlayer = model.passOrder[3];
    model.balls[0].state = ATTACHED;
    model.balls[1].state = ATTACHED;
    model.players[model.passOrder[0]].hasBall = true;
    model.players[model.passOrder[3]].hasBall = true;

    bool running = true;
    while (running) {
        running = handleEvents(&model);

        if (model.currentPage == PAGE_MAIN) {
            updatePassingLogic(&model);
            update_players(model.players);
            for (int i = 0; i < 2; i++) {
                update_ball(&model.balls[i], model.players, &model);
            }            
            //update_ball(&model.ball, model.players, &model);
            renderGame(renderer, textures.playerTexture, textures.grassTexture, &model);
        }
        else if (model.currentPage == PAGE_EMPTY) {
            updateTriangleLogic(&model);
            renderTriangleScene(renderer, &model, textures.playerTexture, textures.triangleTexture);

            // Rita knappar ändå
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_Point triangle[4] = {
                {WINDOW_WIDTH - 60, 10},
                {WINDOW_WIDTH - 30, 10},
                {WINDOW_WIDTH - 45, 30},
                {WINDOW_WIDTH - 60, 10}
            };
            SDL_RenderDrawLines(renderer, triangle, 4);

            SDL_Rect backButton = {WINDOW_WIDTH - 120, 10, 40, 20};
            SDL_RenderDrawRect(renderer, &backButton);

            //SDL_RenderPresent(renderer);
        }

        SDL_Delay(16);
    }

    cleanupModel(&model);
    SDL_DestroyTexture(textures.playerTexture);
    SDL_DestroyTexture(textures.grassTexture);
    SDL_DestroyTexture(textures.coachTexture);
    SDL_DestroyTexture(model.balls[0].texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}


/*void updatePassingLogic(GameModel* model) {
    int from = model->passOrder[0];
    int to = model->passOrder[1];

    Player* fromPlayer = &model->players[from];
    Player* toPlayer = &model->players[to];
    
    movePlayerTowards(fromPlayer, toPlayer->x, toPlayer->y, MOVE_SPEED, model);


    if (fabs(fromPlayer->x - toPlayer->x) < THRESHOLD &&
        fabs(fromPlayer->y - toPlayer->y) < THRESHOLD) {

        //handle_pass(model, from, to);
        fromPlayer->hasBall = 0;
        toPlayer->hasBall = 1;

        int temp = model->passOrder[0];
        for (int i = 0; i < PLAYER_COUNT - 1; i++) {
            model->passOrder[i] = model->passOrder[i + 1];
        }
        model->passOrder[PLAYER_COUNT - 1] = temp;

        
       for (int i = 2; i < PLAYER_COUNT; i++) {
            int current = model->passOrder[i];
            int next = model->passOrder[(i + 1) % PLAYER_COUNT];
            model->players[current].targetX = model->players[next].originalX;
            model->players[current].targetY = model->players[next].originalY;

            PlayerBehavior behavior = model->players[current].behavior;
            if(behavior == BEHAVIOR_CHAIN_MOVE || behavior == BEHAVIOR_ALWAYS_MOVE){

                model->players[current].state = RUN;
                model->players[current].shouldMove = true;

            }
        }
        model->passInitiated = true;
        model->passCompleted = false;

    }

    //updateCoachPosition(toPlayer->x, toPlayer->y, model);
    handle_pass(model, model->passOrder[0], model->passOrder[1]);


    //coach‑rörelse (MANUELL)
    if (model->coachManual){
        float dx = model->coachTargetX - model->coach.x;
        float dy = model->coachTargetY - model->coach.y;
        float dist = sqrtf(dx*dx + dy*dy);

        if (dist < 1.0f) {                
            model->coachManual = false;
            coachIdle(&model->coach);        //sätt IDLE
        } else {
            movePlayerTowards(&model->coach,
                            model->coachTargetX,
                            model->coachTargetY,
                            COACH_SPEED,
                            model);
        }
    }
    else {                            
        coachIdle(&model->coach);           
    }
}*/


void updatePassingLogic(GameModel* model) {
    for (int b = 0; b < 2; b++) {
        Ball* ball = &model->balls[b];
        int currentPlayer = ball->attachedPlayer;

        if (ball->state == ATTACHED && currentPlayer >= 0) {

        
            int currentIndex = -1;
            for (int i = 0; i < PLAYER_COUNT; i++) {
                if (model->passOrder[i] == currentPlayer) {
                    currentIndex = i;
                    break;
                }
            }
            if (currentIndex == -1) continue;

            int nextIndex = (currentIndex + 1) % PLAYER_COUNT;
            int nextPlayer = model->passOrder[nextIndex];

            Player* from = &model->players[currentPlayer];
            Player* to   = &model->players[nextPlayer];

            float targetX = model->cornerX[nextIndex];
            float targetY = model->cornerY[nextIndex];

            movePlayerTowards(from, targetX, targetY, MOVE_SPEED, model);

            if (fabs(from->x - targetX) < THRESHOLD &&
                fabs(from->y - targetY) < THRESHOLD) {


                from->hasBall = 0;
                to->hasBall = 1;
                ball->attachedPlayer = nextPlayer;

                from->x = targetX;
                from->y = targetY;

                from->state = RUN;
                to->state = IDLE;
            }
        }
    }

    if (model->coachManual) {
        float dx = model->coachTargetX - model->coach.x;
        float dy = model->coachTargetY - model->coach.y;
        float dist = sqrtf(dx*dx + dy*dy);
        if (dist < 1.0f) {
            model->coachManual = false;
            coachIdle(&model->coach);
        } else {
            movePlayerTowards(&model->coach,
                              model->coachTargetX,
                              model->coachTargetY,
                              COACH_SPEED,
                              model);
        }
    } else {
        coachIdle(&model->coach);
    }
}


void handle_pass(GameModel* model, int ballIndex, int from, int to) {
    Ball* ball = &model->balls[ballIndex];
    Player* src = &model->players[from];
    Player* dst = &model->players[to];

    src->hasBall = 0;
    dst->hasBall = 1;
    ball->state  = IN_FLIGHT;
    ball->attachedPlayer = -1;

    float dx = dst->x - src->x;
    float dy = dst->y - src->y;
    float dist = sqrtf(dx*dx + dy*dy);

    ball->velX = BALL_SPEED * dx / dist;
    ball->velY = BALL_SPEED * dy / dist;

    src->targetX = dst->x;
    src->targetY = dst->y;
    src->state = RUN;
}

void update_ball(Ball* ball, Player players[], GameModel* model) {
    if (ball->state == IN_FLIGHT) {
        ball->x += ball->velX;
        ball->y += ball->velY;

        int closestPlayer = -1;
        float minDist = 5.0f;

    
        for (int i = 0; i < PLAYER_COUNT; i++) {
            float dx = ball->x - players[i].x;
            float dy = ball->y - players[i].y;
            float dist = sqrtf(dx * dx + dy * dy);

            if (dist < minDist) {
                minDist = dist;
                closestPlayer = i;
            }
        }

        if (closestPlayer != -1) {
            ball->state = ATTACHED;
            ball->attachedPlayer = closestPlayer;
            ball->velX = 0;
            ball->velY = 0;
            players[closestPlayer].hasBall = 1;
            model->passCompleted = true;
        }

        ball->angle += 10.0f;
        if (ball->angle > 360.0f) ball->angle -= 360.0f;
    }else if (ball->state == ATTACHED && ball->attachedPlayer >= 0) {
        ball->x = players[ball->attachedPlayer].x;
        ball->y = players[ball->attachedPlayer].y;
    }
}


void update_players(Player players[PLAYER_COUNT]) {
    for (int i = 0; i < PLAYER_COUNT; i++) {
        Player* p = &players[i];
        if (p->state == RUN) {
            movePlayerTowards(p, p->targetX, p->targetY, MOVE_SPEED, NULL);
            if (fabs(p->x - p->targetX) < THRESHOLD &&
                fabs(p->y - p->targetY) < THRESHOLD) {
                p->x = p->targetX;
                p->y = p->targetY;
                p->state = IDLE;
                p->shouldMove = false;
            }
        }
    }
}
void updateTriangleLogic(GameModel* model) {
    int currentStep = model->triangleStep;
    int from = model->trianglePassOrder[currentStep % 4];
    int to   = model->trianglePassOrder[(currentStep + 1) % 4];

    Player* fromPlayer = &model->trianglePlayers[from];
    Player* toPlayer = &model->trianglePlayers[to];

    movePlayerTowards(fromPlayer, toPlayer->x, toPlayer->y, MOVE_SPEED, NULL);

    if (fabs(fromPlayer->x - toPlayer->x) < THRESHOLD &&
        fabs(fromPlayer->y - toPlayer->y) < THRESHOLD) {

        fromPlayer->hasBall = 0;
        toPlayer->hasBall = 1;

        model->triangleStep++;
    }

    updateTriangleCoach(model);
    
}

void updateTriangleCoach(GameModel* model) {
    if (!model->triangleCoachManual) return;

    float dx = model->triangleCoachTargetX - model->triangleCoach.x;
    float dy = model->triangleCoachTargetY - model->triangleCoach.y;
    float distance = sqrtf(dx * dx + dy * dy);

    float speed = 2.0f;
    if (distance > speed) {
        model->triangleCoach.x += speed * dx / distance;
        model->triangleCoach.y += speed * dy / distance;

        model->triangleCoach.angle = atan2f(dy, dx) * 180.0f / M_PI;
        if (model->triangleCoach.angle < 0) model->triangleCoach.angle += 360;

        model->triangleCoach.animationState = RUN;
    } else {
        model->triangleCoach.x = model->triangleCoachTargetX;
        model->triangleCoach.y = model->triangleCoachTargetY;

        model->triangleCoach.animationState = IDLE;
        model->triangleCoach.frame = 0; // reset to first IDLE frame
        model->triangleCoach.lastFrameTime = SDL_GetTicks(); // optional, to sync
    }
}
