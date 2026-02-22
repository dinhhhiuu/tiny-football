#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "state.h"
#include "config.h"

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

    void renderMenu();
    void renderPlay();
    void renderTimeText();
    void renderField();  // Draw soccer field
    
    // ===== Task B: Player methods =====
    void initPlayers();
    void updatePlayers(float deltaTime);
    void renderPlayers();

    SDL_Window* window;
    SDL_Renderer* renderer;

    bool running;
    GameState state;

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
};