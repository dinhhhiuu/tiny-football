#include "game.h"
#include <iostream>

Game::Game()
    : window(nullptr),
      renderer(nullptr),
      running(true),
      state(GameState::MENU),
      lastTick(0),
      matchTimeLeft(MATCH_TIME_SECONDS) {}

Game::~Game() {}

bool Game::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL Init Failed\n";
        return false;
    }

    window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (!window) return false;

    renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED
    );

    if (TTF_Init() == -1) {
        SDL_Log("TTF init failed");
        return false;
    }

    font = TTF_OpenFont("assets/fonts/ZeroCool.ttf", 24);
    if (!font) {
        SDL_Log("Failed to load font");
        return false;
    }

    if (!renderer) return false;

    return true;
}

void Game::run() {
    lastTick = SDL_GetTicks();

    while (running && state != GameState::EXIT) {
        Uint32 currentTick = SDL_GetTicks();
        float deltaTime = (currentTick - lastTick) / 1000.0f;
        lastTick = currentTick;

        handleEvents();
        update(deltaTime);
        render();

        SDL_Delay(FRAME_DELAY);
    }
}

void Game::handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            state = GameState::EXIT;
        }

        if (e.type == SDL_KEYDOWN) {
            if (state == GameState::MENU) {
                if (e.key.keysym.sym == SDLK_RETURN) {
                    state = GameState::PLAY;
                    matchTimeLeft = MATCH_TIME_SECONDS;
                    lastTick = SDL_GetTicks(); // reset timer
                }
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    state = GameState::EXIT;
                }
            }
            else if (state == GameState::PLAY) {
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    state = GameState::MENU;
                }
            }
        }
    }
}

void Game::update(float deltaTime) {
    if (state == GameState::PLAY) {
        matchTimeLeft -= deltaTime;

        if (matchTimeLeft <= 0.0f) {
            // Hết trận
            state = GameState::MENU; 
            // hoặc state = GameState::EXIT;
        }

        // Sau này:
        // update cầu thủ, bóng = dùng deltaTime
    }
}

void Game::render() {
    SDL_RenderClear(renderer);

    if (state == GameState::MENU) {
        renderMenu();
    }
    else if (state == GameState::PLAY) {
        renderPlay();
    }

    SDL_RenderPresent(renderer);
}

void Game::renderMenu() {
    SDL_SetRenderDrawColor(
        renderer,
        COLOR_MENU_BG_R,
        COLOR_MENU_BG_G,
        COLOR_MENU_BG_B,
        255
    );
    SDL_RenderClear(renderer);

    SDL_Rect box = {
        WINDOW_WIDTH / 2 - 150,
        WINDOW_HEIGHT / 2 - 100,
        300,
        200
    };

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &box);
}

void Game::renderPlay() {
    SDL_SetRenderDrawColor(
        renderer,
        COLOR_FIELD_R,
        COLOR_FIELD_G,
        COLOR_FIELD_B,
        255
    );
    SDL_RenderClear(renderer);

    // vẽ sân + cầu thủ + bóng
    
    renderTimeText();
}

void Game::clean() {
    TTF_CloseFont(font);
    TTF_Quit();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Game::renderTimeText() {
    int timeInt = static_cast<int>(matchTimeLeft);
    if (timeInt < 0) timeInt = 0;

    std::string timeText = "Time: " + std::to_string(timeInt);

    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface* surface =
        TTF_RenderText_Solid(font, timeText.c_str(), white);

    SDL_Texture* texture =
        SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect dst = {20, 20, surface->w, surface->h};

    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, nullptr, &dst);
    SDL_DestroyTexture(texture);
}
