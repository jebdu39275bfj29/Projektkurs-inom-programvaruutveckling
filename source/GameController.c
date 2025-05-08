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

            // Kvadratknapp: gå till ny sida
            else if (mx >= WINDOW_WIDTH - 180 && mx <= WINDOW_WIDTH - 150 && my >= 10 && my <= 30) {
                if (model->currentPage != PAGE_SQUARE) {
                    model->currentPage = PAGE_SQUARE;
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

            // Kvadrat-coachens musklick
            if (!clickedButton && model->currentPage == PAGE_SQUARE) {
                model->squareCoachManual = true;
                model->squareCoachTargetX = (float)mx - PLAYER_SIZE / 2.0f;
                model->squareCoachTargetY = (float)my - PLAYER_SIZE / 2.0f;
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
    initializeModel(&model, textures.coachTexture, textures.ballTexture);
    model.balls[0].texture = textures.ballTexture;
    model.balls[1].texture = textures.ballTexture;
    model.activePlayer = model.passOrder[0];
    model.passInitiated = true;
    model.passCompleted = true;
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
            renderTriangleScene(renderer, &model, textures.playerTexture, textures.grassTexture);

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

        else if (model.currentPage == PAGE_SQUARE) {
            updateSquareBall(&model);
            updateSquareLogic(&model);
            updateSquareCoach(&model);
            renderSquareScene(renderer, &model, textures.grassTexture, textures.playerTexture);
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
            model->passInitiated = true;
            model->passCompleted = false;
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

    float speed = 5.0f;
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


void updateSquareCoach(GameModel* model) {
    if (!model->squareCoachManual) return;

    float dx = model->squareCoachTargetX - model->squareCoach.x;
    float dy = model->squareCoachTargetY - model->squareCoach.y;
    float distance = sqrtf(dx * dx + dy * dy);

    float speed = 5.0f;
    if (distance > speed) {
        model->squareCoach.x += speed * dx / distance;
        model->squareCoach.y += speed * dy / distance;

        model->squareCoach.angle = atan2f(dy, dx) * 180.0f / M_PI;
        if (model->squareCoach.angle < 0) model->squareCoach.angle += 360;

        model->squareCoach.animationState = RUN;

        // ⬇️ Lägg till animation frame-uppdatering här
        Uint32 now = SDL_GetTicks();
        if (now - model->squareCoach.lastFrameTime > FRAME_DELAY) {
            int frameCount = animationFrameCounts[model->squareCoach.animationState];
            model->squareCoach.frame = (model->squareCoach.frame + 1) % frameCount;
            model->squareCoach.lastFrameTime = now;
        }

    } else {
        model->squareCoach.x = model->squareCoachTargetX;
        model->squareCoach.y = model->squareCoachTargetY;

        model->squareCoach.animationState = IDLE;
        model->squareCoach.frame = 0;
        model->squareCoach.lastFrameTime = SDL_GetTicks();
        model->squareCoachManual = false;
    }
}


/*
void updateSquareLogic(GameModel* model) {
    static int playerPosition[5] = {0, 1, 2, 3, 4};  // 5 spelare i rotation
    static int ballTargetIndex = 1;
    static bool firstRun = true;

    // Initierar första spelarens rörelse om det är första uppdateringen
    if (firstRun) {
        int runnerId = playerPosition[0];
        int nextCorner = ballTargetIndex;
        squarePlayers[runnerId].targetX = model->squareBallTargets[nextCorner][0] - PLAYER_SIZE / 2;
        squarePlayers[runnerId].targetY = model->squareBallTargets[nextCorner][1] - PLAYER_SIZE / 2;
        squarePlayers[runnerId].isRunning = true;
        squarePlayers[runnerId].animationState = RUN;
        firstRun = false;
    }

    // Flytta bollen
    model->squareBallX += model->squareBallVelX;
    model->squareBallY += model->squareBallVelY;

    float tx = model->squareBallTargets[ballTargetIndex][0];
    float ty = model->squareBallTargets[ballTargetIndex][1];
    float dx = tx - model->squareBallX;
    float dy = ty - model->squareBallY;
    float dist = sqrtf(dx * dx + dy * dy);

    if (dist < 4.0f) {
        // Uppdatera spelarrotation: medurs skifte
        int temp = playerPosition[4];
        for (int i = 4; i > 0; --i) {
            playerPosition[i] = playerPosition[i - 1];
        }
        playerPosition[0] = temp;

        // Ny spelare ska springa till nästa hörn
        int runnerId = playerPosition[0];
        int nextCorner = (ballTargetIndex + 1) % 4;
        squarePlayers[runnerId].targetX = model->squareBallTargets[nextCorner][0] - PLAYER_SIZE / 2;
        squarePlayers[runnerId].targetY = model->squareBallTargets[nextCorner][1] - PLAYER_SIZE / 2;
        squarePlayers[runnerId].isRunning = true;
        squarePlayers[runnerId].animationState = RUN;

        // Flytta bollen till nästa hörn
        model->squareBallX = tx;
        model->squareBallY = ty;
        float ndx = model->squareBallTargets[nextCorner][0] - model->squareBallX;
        float ndy = model->squareBallTargets[nextCorner][1] - model->squareBallY;
        float len = sqrtf(ndx * ndx + ndy * ndy);
        model->squareBallVelX = (ndx / len) * 5.0f;
        model->squareBallVelY = (ndy / len) * 5.0f;

        ballTargetIndex = nextCorner;
    }

    // Flytta springande spelare
    for (int i = 0; i < 5; ++i) {
        if (squarePlayers[i].isRunning) {
            movePlayerTowards(&squarePlayers[i], squarePlayers[i].targetX, squarePlayers[i].targetY, 4.0f, model);

            float ddx = squarePlayers[i].targetX - squarePlayers[i].x;
            float ddy = squarePlayers[i].targetY - squarePlayers[i].y;
            if (fabsf(ddx) < 2.0f && fabsf(ddy) < 2.0f) {
                squarePlayers[i].x = squarePlayers[i].targetX;
                squarePlayers[i].y = squarePlayers[i].targetY;
                squarePlayers[i].isRunning = false;
                squarePlayers[i].animationState = IDLE;
            }
        }
    }
}
*/


void updateSquareLogic(GameModel* model) {
    static const int targetCorners[4] = {1, 2, 3, 0}; // UR → BR → BL → UL
    static int playerPositions[5] = {4, 0, 3, 2, 1};  // Medurs: UL, UR, BR, BL, och utanför
    static int ballTargetIndex = 1;                   // Bollen går till hörn 1 (UR) först
    static bool firstRun = true;

    float dx = model->squareBallTargets[ballTargetIndex][0] - model->squareBallX;
    float dy = model->squareBallTargets[ballTargetIndex][1] - model->squareBallY;
    float dist = sqrtf(dx * dx + dy * dy);

    // Initiera första löparen bara en gång
    if (firstRun) {
    for (int i = 0; i < SQUARE_PLAYER_COUNT; i++) {
        squarePlayers[i].isRunning = false;
        squarePlayers[i].targetX = squarePlayers[i].x;
        squarePlayers[i].targetY = squarePlayers[i].y;
    }

    // ⬅️ Första löparen är playerPositions[0] = 4
    int runnerId = playerPositions[0];
    int targetCorner = model->squareBallTargetIndex; // ex: 1 = UR
    squarePlayers[runnerId].targetX = model->squareBallTargets[targetCorner][0] - PLAYER_SIZE / 2;
    squarePlayers[runnerId].targetY = model->squareBallTargets[targetCorner][1] - PLAYER_SIZE / 2;
    squarePlayers[runnerId].isRunning = true;
    squarePlayers[runnerId].animationState = RUN;
    firstRun = false;
}


    // Flytta bollen
    model->squareBallX += model->squareBallVelX;
    model->squareBallY += model->squareBallVelY;

    if (dist < 4.0f) {
        // När bollen når hörnet: rotera spelarna
        int last = playerPositions[4];
        for (int i = 4; i > 0; --i) {
            playerPositions[i] = playerPositions[i - 1];
        }
        playerPositions[0] = last;

        // Nytt mål för bollen
        ballTargetIndex = (ballTargetIndex + 1) % 4;

        // Starta nästa löpare
        int runnerId = playerPositions[0];
        squarePlayers[runnerId].targetX = model->squareBallTargets[ballTargetIndex][0] - PLAYER_SIZE / 2;
        squarePlayers[runnerId].targetY = model->squareBallTargets[ballTargetIndex][1] - PLAYER_SIZE / 2;
        squarePlayers[runnerId].isRunning = true;
        squarePlayers[runnerId].animationState = RUN;

        // Snappa bollen till nuvarande hörn
        int previousIndex = (ballTargetIndex + 3) % 4;
        model->squareBallX = model->squareBallTargets[previousIndex][0];
        model->squareBallY = model->squareBallTargets[previousIndex][1];

        // Räkna ut ny riktning för bollen mot nästa hörn
        int nextCorner = ballTargetIndex;
        float ndx = model->squareBallTargets[nextCorner][0] - model->squareBallX;
        float ndy = model->squareBallTargets[nextCorner][1] - model->squareBallY;

        float len = sqrtf(ndx * ndx + ndy * ndy);
        if (len > 0.1f) {
            model->squareBallVelX = (ndx / len) * 6.0f;
            model->squareBallVelY = (ndy / len) * 6.0f;
        }
    }

    // Uppdatera spelarna
    for (int i = 0; i < SQUARE_PLAYER_COUNT; i++) {
        if (squarePlayers[i].isRunning) {
            movePlayerTowards(&squarePlayers[i], squarePlayers[i].targetX, squarePlayers[i].targetY, MOVE_SPEED/4, model);
        }
    }
}
