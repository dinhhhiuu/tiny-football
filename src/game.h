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

    SDL_Window* window;
    SDL_Renderer* renderer;

    bool running;
    GameState state;

    // ===== Time =====
    Uint32 lastTick;
    float matchTimeLeft;

    // ===== Text =====
    TTF_Font* font;
};
