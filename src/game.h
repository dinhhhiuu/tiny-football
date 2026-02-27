#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "state.h"
#include "config.h"
#include "ball.h"
#include "collision.h"
#include "score.h"

class Game {
public:
    Game();
    ~Game();

    bool init();
    void run();
    void clean();

private:
    void handleEvents();
    void update(float deltaTime);
    void render();

    void renderStart();
    void renderMenu();
    void renderPlay();
    void renderResults();
    void renderTimeText();
    void renderField();  // Draw soccer field
    
    // ===== Task B: Player methods =====
    void initPlayers();
    void updatePlayers(float deltaTime);
    void updateAI(float deltaTime);
    void checkBallInCorner();//
    void renderPlayers();
    
    // ===== Task C: Ball methods =====
    void initBall();
    void updateBall(float deltaTime);
    void renderBall();
    
    // ===== Task C: Score methods =====
    void renderScore();

    SDL_Window* window;
    SDL_Renderer* renderer;

    bool running;
    GameState state;
    GameMode mode;

    // ===== Time =====
    Uint32 lastTick;
    float matchTimeLeft;

    // ===== Text =====
    TTF_Font* font;
    
    // ===== Task B: Players =====
    Player players[TOTAL_PLAYERS];  // 6 players (3v3)
    int activePlayerTeam1;  // Index of active player in Team 1 (0-2)
    int activePlayerTeam2;  // Index of active player in Team 2 (3-5)
    
    // ===== Task B: Keyboard state (event-based) =====
    bool keyW, keyA, keyS, keyD;
    bool keyUp, keyDown, keyLeft, keyRight;
    
    // ===== Task C: Ball and Score =====
    Ball ball;
    Score score;
};
