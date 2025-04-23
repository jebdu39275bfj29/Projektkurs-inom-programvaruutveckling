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
    model.ball.texture = textures.ballTexture;
    //model.activePlayer = model.passOrder[0];
    model.passInitiated = true;
    bool running = true;
    model.passCompleted = true;
    handle_pass(&model, model.passOrder[0], model.passOrder[1]);
    while (running) {
        running = handleEvents(&model);

        if (model.currentPage == PAGE_MAIN) {
            updatePassingLogic(&model);
            update_players(model.players);
            update_ball(&model.ball, model.players, &model);
            renderGame(renderer, textures.playerTexture, textures.grassTexture, &model);
        }
        else if (model.currentPage == PAGE_EMPTY) {
            SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255); // mörkgrå bakgrund
            SDL_RenderClear(renderer);

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

            SDL_RenderPresent(renderer);
        }

        SDL_Delay(16);
    }

    cleanupModel(&model);
    SDL_DestroyTexture(textures.playerTexture);
    SDL_DestroyTexture(textures.grassTexture);
    SDL_DestroyTexture(textures.coachTexture);
    SDL_DestroyTexture(model.ball.texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}


void updatePassingLogic(GameModel* model) {
    int from = model->passOrder[0];
    int to = model->passOrder[1];

    Player* fromPlayer = &model->players[from];
    Player* toPlayer = &model->players[to];
    
    movePlayerTowards(fromPlayer, toPlayer->x, toPlayer->y, MOVE_SPEED, model);


    if (fabs(fromPlayer->x - toPlayer->x) < THRESHOLD &&
        fabs(fromPlayer->y - toPlayer->y) < THRESHOLD) {

        fromPlayer->hasBall = 0;
        toPlayer->hasBall = 1;

        int temp = model->passOrder[0];
        for (int i = 0; i < PLAYER_COUNT - 1; i++) {
            model->passOrder[i] = model->passOrder[i + 1];
        }
        model->passOrder[PLAYER_COUNT - 1] = temp;

        
       /*for (int i = 2; i < PLAYER_COUNT; i++) {
            int current = model->passOrder[i];
            int next = model->passOrder[(i + 1) % PLAYER_COUNT];
            model->players[current].targetX = model->players[next].originalX;
            model->players[current].targetY = model->players[next].originalY;
            model->players[current].state = RUN;
        }*/
        model->passInitiated = false;
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


}


/*void updateCoachPosition(float targetX, float targetY, GameModel *model) {
    
    //float coachTargetX = targetX - 300;
    //float coachTargetY = targetY - 200;

    float offset = 150.0f;
    float angle = atan2f(targetY - model->coach.y, targetX - model->coach.x);
    float coachTargetX = targetX + cosf(angle + M_PI) * offset;
    float coachTargetY = targetY + sinf(angle + M_PI) * offset;

    
    coachTargetX = fmax(model->grass.x, fmin(coachTargetX, model->grass.x + model->grass.width));
    coachTargetY = fmax(model->grass.y, fmin(coachTargetY, model->grass.y + model->grass.height));

    
    //coachTargetX += 100;
    //coachTargetY += 100;

    
    float dx = coachTargetX - model->coach.x;
    float dy = coachTargetY - model->coach.y;
    float distance = sqrtf(dx * dx + dy * dy);

    
    if (distance > 1.0f) {
        model->coach.animationState = RUN;
    } else {
        model->coach.animationState = IDLE;
    }

    movePlayerTowards(&model->coach, coachTargetX, coachTargetY, MOVE_SPEED, model);
}*/

void handle_pass(GameModel* model, int from, int to) {
    Player* fromPlayer = &model->players[from];
    Player* toPlayer = &model->players[to];
    
    fromPlayer->hasBall = 0;
    model->ball.state = IN_FLIGHT;
    model->ball.attachedPlayer = -1;

    float dx = toPlayer->x - fromPlayer->x;
    float dy = toPlayer->y - fromPlayer->y;
    float dist = sqrtf(dx * dx + dy * dy);
    
    model->ball.velX = BALL_SPEED * dx / dist;
    model->ball.velY = BALL_SPEED * dy / dist;

    fromPlayer->targetX = toPlayer->x;
    fromPlayer->targetY = toPlayer->y;
    fromPlayer->state = RUN;
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
        if (players[i].state == RUN) {
            movePlayerTowards(&players[i], players[i].targetX, players[i].targetY, MOVE_SPEED, NULL);
            if (fabs(players[i].x - players[i].targetX) < THRESHOLD &&
                fabs(players[i].y - players[i].targetY) < THRESHOLD) {
                players[i].x = players[i].targetX;
                players[i].y = players[i].targetY;
                players[i].state = IDLE;
            }
        }
    }
}