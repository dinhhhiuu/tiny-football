#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <stdbool.h>

class Game {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;

public:
    Game();
    ~Game();

    bool init();
    void handleEvents();
    void update();
    void render();
    void clean();

    void run();

    SDL_Renderer* getRenderer();
};

#endif
