#include "game.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <string>

// Timer tracking how long the ball has been in a corner

Game::Game()
    : window(nullptr),
      renderer(nullptr),
      running(true),
      state(GameState::START),
      mode(GameMode::PVP),
      lastTick(0),
      matchTimeLeft(MATCH_TIME_SECONDS),
      activePlayerTeam1(0),  // Start with first player of Team 1
      activePlayerTeam2(3),  // Start with first player of Team 2
      keyW(false), keyA(false), keyS(false), keyD(false),
      keyUp(false), keyDown(false), keyLeft(false), keyRight(false) {}

Game::~Game() {}

bool Game::init() {
    std::cout << "Initializing SDL...\n";
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL Init Failed: " << SDL_GetError() << "\n";
        return false;
    }
    std::cout << "SDL initialized successfully\n";

    std::cout << "Creating window...\n";
    window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cout << "Window creation failed: " << SDL_GetError() << "\n";
        return false;
    }
    std::cout << "Window created successfully\n";

    renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED
    );

    if (TTF_Init() == -1) {
        SDL_Log("TTF init failed: %s\n", TTF_GetError());
        std::cout << "Warning: TTF init failed, continuing without text\n";
        font = nullptr;
    } else {
        font = TTF_OpenFont("assets/fonts/ZeroCool.ttf", 24);
        if (!font) {
            SDL_Log("Failed to load font: %s\n", TTF_GetError());
            std::cout << "Warning: Font not loaded, continuing without text\n";
        }
    }

    if (!renderer) return false;

    // Task B: Initialize players
    initPlayers();
    
    // Task C: Initialize ball and score
    initBall();
    score.reset();

    std::cout << "\n=================================\n";
    std::cout << "    TINY FOOTBALL - Ready!\n";
    std::cout << "=================================\n";
    std::cout << "Currently at START screen.\n";
    std::cout << "Press ENTER to start playing\n";
    std::cout << "Press ESC to exit\n\n";

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
    static bool firstEvent = true;
    if (firstEvent) {
        std::cout << "[DEBUG] handleEvents is running!\n";
        firstEvent = false;
    }
    
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            state = GameState::EXIT;
        }
        
        // Debug: Log keyboard events
        if (e.type == SDL_KEYDOWN) {
            std::cout << "[EVENT] Key pressed: " << SDL_GetKeyName(e.key.keysym.sym) << "\n";
        }
        
        // ===== Task B: Track WASD and Arrow keys =====
        if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
            bool isDown = (e.type == SDL_KEYDOWN);
            
            switch (e.key.keysym.sym) {
                case SDLK_w: keyW = isDown; break;
                case SDLK_a: keyA = isDown; break;
                case SDLK_s: keyS = isDown; break;
                case SDLK_d: keyD = isDown; break;
                case SDLK_UP: keyUp = isDown; break;
                case SDLK_DOWN: keyDown = isDown; break;
                case SDLK_LEFT: keyLeft = isDown; break;
                case SDLK_RIGHT: keyRight = isDown; break;
                
                // TAB: Switch player in Team 1
                case SDLK_TAB:
                    if (e.type == SDL_KEYDOWN && state == GameState::PLAY) {
                        players[activePlayerTeam1].isActive = false;
                        activePlayerTeam1 = (activePlayerTeam1 + 1) % PLAYERS_PER_TEAM;
                        players[activePlayerTeam1].isActive = true;
                        std::cout << "[SWITCH] Team 1 now controls Player " << (activePlayerTeam1 + 1) << "\n";
                    }
                    break;
                    
                // SHIFT: Switch player in Team 2
                case SDLK_RSHIFT:
                case SDLK_LSHIFT:
                    if (e.type == SDL_KEYDOWN && state == GameState::PLAY) {
                        players[activePlayerTeam2].isActive = false;
                        activePlayerTeam2 = 3 + ((activePlayerTeam2 - 3 + 1) % PLAYERS_PER_TEAM);
                        players[activePlayerTeam2].isActive = true;
                        std::cout << "[SWITCH] Team 2 now controls Player " << (activePlayerTeam2 - 2) << "\n";
                    }
                    break;
            }
        }

        if (e.type == SDL_KEYDOWN) {
            if (state == GameState::START) {
                if (e.key.keysym.sym == SDLK_RETURN) {
                    state = GameState::MENU;
                    std::cout << "Choose game mode .\n";
                }
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    state = GameState::EXIT;
                }
            }
            else if (state == GameState::MENU) {
                // Mode selection: 1 = PVP, 2 = PVE
                if (e.key.keysym.sym == SDLK_1) {
                    mode = GameMode::PVP;
                    std::cout << "[MODE] PVP selected\n";
                } else if (e.key.keysym.sym == SDLK_2) {
                    mode = GameMode::PVE;
                    std::cout << "[MODE] PVE selected\n";
                }

                if (e.key.keysym.sym == SDLK_RETURN) {
                    state = GameState::PLAY;
                    matchTimeLeft = MATCH_TIME_SECONDS;
                    lastTick = SDL_GetTicks(); // reset timer
                    initPlayers(); // Task B: Reset players
                    initBall();    // Task C: Reset ball
                    score.reset(); // Task C: Reset score
                    // If PVE selected, make Team 1 AI (disable player input indicator)
                    if (mode == GameMode::PVE) {
                        for (int i = 0; i < PLAYERS_PER_TEAM; ++i) {
                            players[i].isActive = false; // team1 AI controls
                            players[i].isAI = true;
                        }
                        // Ensure Team2 has an active player for human control
                        activePlayerTeam2 = 3;
                        players[activePlayerTeam2].isActive = true;
                    } else {
                        // clear AI flags in PVP
                        for (int i = 0; i < PLAYERS_PER_TEAM; ++i) players[i].isAI = false;
                    }
                    std::cout << "\n=== GAME STARTED (3v3) ===\n";
                    std::cout << "Selected mode: " << (mode == GameMode::PVP ? "PVP" : "PVE") << "\n";
                    std::cout << "Team 1 (BLUE): Use W A S D to move\n";
                    std::cout << "  Press TAB to switch player\n";
                    std::cout << "Team 2 (RED): Use Arrow Keys to move\n";
                    std::cout << "  Press SHIFT to switch player\n";
                    std::cout << "Active players have YELLOW circle\n";
                    std::cout << "Press ESC to return to menu\n\n";
                }
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    state = GameState::EXIT;
                }
            }
            else if (state == GameState::PLAY) {
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    state = GameState::START;
                }
            }
            else if (state == GameState::RESULTS) {
                if (e.key.keysym.sym == SDLK_RETURN) {
                    state = GameState::MENU; // Back to mode selection
                }
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    state = GameState::EXIT;
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
            matchTimeLeft = 0.0f;
            state = GameState::RESULTS; // show results screen
        }

        // Task B: Update players
        updatePlayers(deltaTime);
        
        // Task C: Update ball
        updateBall(deltaTime);
    }
}

void Game::render() {
    SDL_RenderClear(renderer);

    if (state == GameState::START) {
        renderStart();
    }
    else if (state == GameState::MENU) {
        renderMenu();
    }
    else if (state == GameState::PLAY) {
        renderPlay();
    }
    else if (state == GameState::RESULTS) {
        renderResults();
    }

    SDL_RenderPresent(renderer);
}

void Game::renderStart() {
        SDL_SetRenderDrawColor(
        renderer,
        COLOR_BG_R,
        COLOR_BG_G,
        COLOR_BG_B,
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
    
    if (!font) return; // Skip text if font not loaded
    
    // Menu text
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Solid(font, "TINY FOOTBALL", white);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect titleRect = {WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2 - 60, 160, 30};
    SDL_RenderCopy(renderer, texture, nullptr, &titleRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    
    surface = TTF_RenderText_Solid(font, "Press ENTER to Play", white);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect enterRect = {WINDOW_WIDTH / 2 - 90, WINDOW_HEIGHT / 2, 180, 25};
    SDL_RenderCopy(renderer, texture, nullptr, &enterRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    
    surface = TTF_RenderText_Solid(font, "ESC to Exit", white);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect escRect = {WINDOW_WIDTH / 2 - 60, WINDOW_HEIGHT / 2 + 40, 120, 25};
    SDL_RenderCopy(renderer, texture, nullptr, &escRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void Game::renderMenu() {
    SDL_SetRenderDrawColor(
        renderer,
        COLOR_BG_R,
        COLOR_BG_G,
        COLOR_BG_B,
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
    
    if (!font) return; // Skip text if font not loaded

    // Menu text
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Solid(font, "TINY FOOTBALL", white);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect titleRect = {WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2 - 90, 160, 30};
    SDL_RenderCopy(renderer, texture, nullptr, &titleRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // Options: 1 = PVP, 2 = PVE
    std::string opt1 = (mode == GameMode::PVP) ? "> 1: PVP (Player vs Player)" : "  1: PVP (Player vs Player)";
    std::string opt2 = (mode == GameMode::PVE) ? "> 2: PVE (Player vs AI)" : "  2: PVE (Player vs AI)";

    surface = TTF_RenderText_Solid(font, opt1.c_str(), white);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect o1Rect = {WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 - 40, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &o1Rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    surface = TTF_RenderText_Solid(font, opt2.c_str(), white);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect o2Rect = {WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 - 10, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &o2Rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // Start / exit hints
    surface = TTF_RenderText_Solid(font, "Press ENTER to Start", white);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect enterRect = {WINDOW_WIDTH / 2 - 90, WINDOW_HEIGHT / 2 + 30, 180, 25};
    SDL_RenderCopy(renderer, texture, nullptr, &enterRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    surface = TTF_RenderText_Solid(font, "ESC to Exit", white);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect escRect = {WINDOW_WIDTH / 2 - 60, WINDOW_HEIGHT / 2 + 60, 120, 25};
    SDL_RenderCopy(renderer, texture, nullptr, &escRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
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

    // Draw soccer field
    renderField();

    // Task B: Render players
    renderPlayers();
    
    // Task C: Render ball and score
    renderBall();
    renderScore();
    
    renderTimeText();
    
    if (!font) return; // Skip hints if font not loaded
    
    // Player controls hint
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Solid(font, "P1: WASD", white);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect p1Rect = {10, WINDOW_HEIGHT - 30, 100, 20};
    SDL_RenderCopy(renderer, texture, nullptr, &p1Rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    
    surface = TTF_RenderText_Solid(font, "P2: Arrows", white);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect p2Rect = {WINDOW_WIDTH - 120, WINDOW_HEIGHT - 30, 110, 20};
    SDL_RenderCopy(renderer, texture, nullptr, &p2Rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void Game::renderResults() {
    SDL_SetRenderDrawColor(
        renderer,
        COLOR_BG_R,
        COLOR_BG_G,
        COLOR_BG_B,
        255
    );
    SDL_RenderClear(renderer);

    SDL_Rect box = {
        WINDOW_WIDTH / 2 - 200,
        WINDOW_HEIGHT / 2 - 120,
        400,
        240
    };

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &box);

    if (!font) return;

    SDL_Color white = {255, 255, 255, 255};

    // Title
    SDL_Surface* surface = TTF_RenderText_Solid(font, "MATCH ENDED", white);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect titleRect = {WINDOW_WIDTH / 2 - surface->w / 2, WINDOW_HEIGHT / 2 - 90, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &titleRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // Score
    std::string scoreText = "Team 1: " + std::to_string(score.team1Score) + "  -  " + "Team 2: " + std::to_string(score.team2Score);
    surface = TTF_RenderText_Solid(font, scoreText.c_str(), white);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect scoreRect = {WINDOW_WIDTH / 2 - surface->w / 2, WINDOW_HEIGHT / 2 - 30, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &scoreRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // Winner text
    std::string winner;
    if (score.team1Score > score.team2Score) winner = "Team 1 (BLUE) wins!";
    else if (score.team2Score > score.team1Score) winner = "Team 2 (RED) wins!";
    else winner = "Draw!";

    surface = TTF_RenderText_Solid(font, winner.c_str(), white);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect winRect = {WINDOW_WIDTH / 2 - surface->w / 2, WINDOW_HEIGHT / 2 + 10, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &winRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // Hints
    surface = TTF_RenderText_Solid(font, "Press ENTER to return to menu", white);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect hintRect = {WINDOW_WIDTH / 2 - surface->w / 2, WINDOW_HEIGHT / 2 + 60, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &hintRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    surface = TTF_RenderText_Solid(font, "Press ESC to exit", white);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect escRect = {WINDOW_WIDTH / 2 - surface->w / 2, WINDOW_HEIGHT / 2 + 90, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &escRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void Game::clean() {
    TTF_CloseFont(font);
    TTF_Quit();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Game::renderTimeText() {
    if (!font) return; // Skip if font not loaded
    
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

void Game::renderField() {
    // Use field constants from config.h
    const int fieldLeft = FIELD_LEFT;
    const int fieldRight = FIELD_RIGHT;
    const int fieldTop = FIELD_TOP;
    const int fieldBottom = FIELD_BOTTOM;
    const int fieldWidth = fieldRight - fieldLeft;
    const int fieldHeight = fieldBottom - fieldTop;
    const int centerX = WINDOW_WIDTH / 2;
    const int centerY = WINDOW_HEIGHT / 2;
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // White lines
    
    // ===== Outer boundary =====
    SDL_Rect boundary = {fieldLeft, fieldTop, fieldWidth, fieldHeight};
    SDL_RenderDrawRect(renderer, &boundary);
    
    // ===== Center line =====
    SDL_RenderDrawLine(renderer, centerX, fieldTop, centerX, fieldBottom);
    
    // ===== Center circle =====
    int centerRadius = 60;
    for (int angle = 0; angle < 360; angle += 5) {
        float rad1 = angle * 3.14159f / 180.0f;
        float rad2 = (angle + 5) * 3.14159f / 180.0f;
        int x1 = centerX + (int)(centerRadius * cos(rad1));
        int y1 = centerY + (int)(centerRadius * sin(rad1));
        int x2 = centerX + (int)(centerRadius * cos(rad2));
        int y2 = centerY + (int)(centerRadius * sin(rad2));
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }
    
    // ===== Center spot =====
    SDL_Rect centerSpot = {centerX - 3, centerY - 3, 6, 6};
    SDL_RenderFillRect(renderer, &centerSpot);
    
    // ===== Left goal =====
    int goalWidth = 80;
    int goalDepth = 20;
    SDL_Rect leftGoal = {fieldLeft - goalDepth, centerY - goalWidth/2, goalDepth, goalWidth};
    SDL_RenderDrawRect(renderer, &leftGoal);
    
    // Left penalty area
    int penaltyWidth = 160;
    int penaltyDepth = 60;
    SDL_Rect leftPenalty = {fieldLeft, centerY - penaltyWidth/2, penaltyDepth, penaltyWidth};
    SDL_RenderDrawRect(renderer, &leftPenalty);
    
    // Left penalty spot
    SDL_Rect leftSpot = {fieldLeft + 40, centerY - 3, 6, 6};
    SDL_RenderFillRect(renderer, &leftSpot);
    
    // ===== Right goal =====
    SDL_Rect rightGoal = {fieldRight, centerY - goalWidth/2, goalDepth, goalWidth};
    SDL_RenderDrawRect(renderer, &rightGoal);
    
    // Right penalty area
    SDL_Rect rightPenalty = {fieldRight - penaltyDepth, centerY - penaltyWidth/2, penaltyDepth, penaltyWidth};
    SDL_RenderDrawRect(renderer, &rightPenalty);
    
    // Right penalty spot
    SDL_Rect rightSpot = {fieldRight - 40, centerY - 3, 6, 6};
    SDL_RenderFillRect(renderer, &rightSpot);
    
    // ===== Corner marks (thicker filled marks to ensure visibility) =====
    int cornerSize = 10;
    int thickness = 2;

    // Top-left
    SDL_Rect tlH = { fieldLeft, fieldTop, cornerSize, thickness };
    SDL_Rect tlV = { fieldLeft, fieldTop, thickness, cornerSize };
    SDL_RenderFillRect(renderer, &tlH);
    SDL_RenderFillRect(renderer, &tlV);

    // Top-right
    SDL_Rect trH = { fieldRight - cornerSize, fieldTop, cornerSize, thickness };
    SDL_Rect trV = { fieldRight - thickness, fieldTop, thickness, cornerSize };
    SDL_RenderFillRect(renderer, &trH);
    SDL_RenderFillRect(renderer, &trV);

    // Bottom-left
    SDL_Rect blH = { fieldLeft, fieldBottom - thickness, cornerSize, thickness };
    SDL_Rect blV = { fieldLeft, fieldBottom - cornerSize, thickness, cornerSize };
    SDL_RenderFillRect(renderer, &blH);
    SDL_RenderFillRect(renderer, &blV);

    // Bottom-right
    SDL_Rect brH = { fieldRight - cornerSize, fieldBottom - thickness, cornerSize, thickness };
    SDL_Rect brV = { fieldRight - thickness, fieldBottom - cornerSize, thickness, cornerSize };
    SDL_RenderFillRect(renderer, &brH);
    SDL_RenderFillRect(renderer, &brV);
}

// ===== TASK B: PLAYER IMPLEMENTATION =====

void Game::initPlayers() {
    // ===== TEAM 1 (Blue) - Left side - Controlled by WASD =====
    // Defender
    players[0].x = WINDOW_WIDTH / 6.0f;
    players[0].y = WINDOW_HEIGHT / 2.0f;
    players[0].w = PLAYER_SIZE;
    players[0].h = PLAYER_SIZE;
    players[0].speed = PLAYER_SPEED;
    players[0].playerID = 0;
    players[0].teamID = 0;
    players[0].isActive = true;
    players[0].r = 50;
    players[0].g = 100;
    players[0].b = 255;
    
    // Midfielder
    players[1].x = WINDOW_WIDTH / 3.5f;
    players[1].y = WINDOW_HEIGHT / 3.0f;
    players[1].w = PLAYER_SIZE;
    players[1].h = PLAYER_SIZE;
    players[1].speed = PLAYER_SPEED;
    players[1].playerID = 1;
    players[1].teamID = 0;
    players[1].isActive = false;
    players[1].r = 50;
    players[1].g = 100;
    players[1].b = 255;
    
    // Forward
    players[2].x = WINDOW_WIDTH / 3.5f;
    players[2].y = WINDOW_HEIGHT * 2.0f / 3.0f;
    players[2].w = PLAYER_SIZE;
    players[2].h = PLAYER_SIZE;
    players[2].speed = PLAYER_SPEED;
    players[2].playerID = 2;
    players[2].teamID = 0;
    players[2].isActive = false;
    players[2].r = 50;
    players[2].g = 100;
    players[2].b = 255;
    
    // ===== TEAM 2 (Red) - Right side - Controlled by Arrow Keys =====
    // Defender
    players[3].x = WINDOW_WIDTH * 5.0f / 6.0f;
    players[3].y = WINDOW_HEIGHT / 2.0f;
    players[3].w = PLAYER_SIZE;
    players[3].h = PLAYER_SIZE;
    players[3].speed = PLAYER_SPEED;
    players[3].playerID = 3;
    players[3].teamID = 1;
    players[3].isActive = true;
    players[3].r = 255;
    players[3].g = 50;
    players[3].b = 50;
    
    // Midfielder
    players[4].x = WINDOW_WIDTH * 2.5f / 3.5f;
    players[4].y = WINDOW_HEIGHT / 3.0f;
    players[4].w = PLAYER_SIZE;
    players[4].h = PLAYER_SIZE;
    players[4].speed = PLAYER_SPEED;
    players[4].playerID = 4;
    players[4].teamID = 1;
    players[4].isActive = false;
    players[4].r = 255;
    players[4].g = 50;
    players[4].b = 50;
    
    // Forward
    players[5].x = WINDOW_WIDTH * 2.5f / 3.5f;
    players[5].y = WINDOW_HEIGHT * 2.0f / 3.0f;
    players[5].w = PLAYER_SIZE;
    players[5].h = PLAYER_SIZE;
    players[5].speed = PLAYER_SPEED;
    players[5].playerID = 5;
    players[5].teamID = 1;
    players[5].isActive = false;
    players[5].r = 255;
    players[5].g = 50;
    players[5].b = 50;
    
    activePlayerTeam1 = 0;
    activePlayerTeam2 = 3;

    // Initialize velocity and prev positions
    for (int i = 0; i < TOTAL_PLAYERS; ++i) {
        players[i].vx = 0.0f;
        players[i].vy = 0.0f;
        players[i].prevX = players[i].x;
        players[i].prevY = players[i].y;
        players[i].isAI = false;
    }
}

void Game::updatePlayers(float deltaTime) {
    // CHANGED: Use event-based keyboard state instead of SDL_GetKeyboardState
    // This fixes the issue where W A S D keys were not detected
    
    // Debug: Check if any key is pressed
    static int debugCounter = 0;
    static bool firstCheck = true;
    if (firstCheck) {
        std::cout << "\n[DEBUG] updatePlayers using EVENT-BASED input!\n";
        std::cout << "[DEBUG] 3v3 game! Use TAB (Team 1) and SHIFT (Team 2) to switch players\n";
        firstCheck = false;
    }
    
    if (debugCounter++ % 30 == 0) { // Print every 30 frames (0.5 seconds)
        if (keyW || keyA || keyS || keyD) {
            std::cout << "[INPUT] Team 1 Player " << (activePlayerTeam1 + 1) << " keys: ";
            if (keyW) std::cout << "W ";
            if (keyA) std::cout << "A ";
            if (keyS) std::cout << "S ";
            if (keyD) std::cout << "D ";
            std::cout << "\n";
        }
    }
    
    // Save previous positions for velocity calculation
    for (int i = 0; i < TOTAL_PLAYERS; ++i) {
        players[i].prevX = players[i].x;
        players[i].prevY = players[i].y;
    }

    // ===== Team 1 (Blue) =====
    if (mode == GameMode::PVE) {
        // AI controls Team 1 in PVE mode
        updateAI(deltaTime);
    } else {
        // Human controls Team 1 (W A S D) - only active player
        int p1 = activePlayerTeam1;
        if (keyW) {
            players[p1].y -= players[p1].speed * deltaTime;
        }
        if (keyS) {
            players[p1].y += players[p1].speed * deltaTime;
        }
        if (keyA) {
            players[p1].x -= players[p1].speed * deltaTime;
        }
        if (keyD) {
            players[p1].x += players[p1].speed * deltaTime;
        }
    }
    
    // ===== Team 2 (Red): Arrow Keys - Only active player =====
    int p2 = activePlayerTeam2;
    if (keyUp) {
        players[p2].y -= players[p2].speed * deltaTime;
    }
    if (keyDown) {
        players[p2].y += players[p2].speed * deltaTime;
    }
    if (keyLeft) {
        players[p2].x -= players[p2].speed * deltaTime;
    }
    if (keyRight) {
        players[p2].x += players[p2].speed * deltaTime;
    }
    
    // ===== Keep players inside the FIELD (not window) =====
    for (int i = 0; i < TOTAL_PLAYERS; i++) {
        if (players[i].x < FIELD_LEFT) {
            players[i].x = FIELD_LEFT;
        }
        if (players[i].x + players[i].w > FIELD_RIGHT) {
            players[i].x = FIELD_RIGHT - players[i].w;
        }
        if (players[i].y < FIELD_TOP) {
            players[i].y = FIELD_TOP;
        }
        if (players[i].y + players[i].h > FIELD_BOTTOM) {
            players[i].y = FIELD_BOTTOM - players[i].h;
        }
    }

    // Compute player velocities from position changes
    for (int i = 0; i < TOTAL_PLAYERS; ++i) {
        players[i].vx = (players[i].x - players[i].prevX) / (deltaTime > 0.0001f ? deltaTime : 0.0001f);
        players[i].vy = (players[i].y - players[i].prevY) / (deltaTime > 0.0001f ? deltaTime : 0.0001f);
    }
}

// Draw players
void Game::renderPlayers() {
    static bool firstRender = true;
    if (firstRender) {
        std::cout << "[DEBUG] renderPlayers called! 3v3 game\n";
        std::cout << "[DEBUG] Team 1 (Blue): 3 players\n";
        std::cout << "[DEBUG] Team 2 (Red): 3 players\n";
        firstRender = false;
    }
    
    for (int i = 0; i < TOTAL_PLAYERS; i++) {
        int centerX = players[i].x + players[i].w / 2;
        
        // Set player color
        SDL_SetRenderDrawColor(renderer, players[i].r, players[i].g, players[i].b, 255);
        
        // ===== HEAD (circle with face color) =====
        int headRadius = 7;
        int headY = players[i].y + 10;
        
        // Skin color for head
        SDL_SetRenderDrawColor(renderer, 255, 220, 177, 255); // Skin tone
        for (int y = -headRadius; y <= headRadius; y++) {
            for (int x = -headRadius; x <= headRadius; x++) {
                if (x*x + y*y <= headRadius*headRadius) {
                    SDL_RenderDrawPoint(renderer, centerX + x, headY + y);
                }
            }
        }
        
        // Eyes
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawPoint(renderer, centerX - 3, headY - 2);
        SDL_RenderDrawPoint(renderer, centerX + 3, headY - 2);
        
        // ===== JERSEY (rectangle with number) =====
        int jerseyTop = headY + headRadius;
        int jerseyBottom = players[i].y + players[i].h - 12;
        int jerseyLeft = centerX - 10;
        int jerseyRight = centerX + 10;
        
        // Draw jersey body
        SDL_SetRenderDrawColor(renderer, players[i].r, players[i].g, players[i].b, 255);
        SDL_Rect jersey = {jerseyLeft, jerseyTop, 20, jerseyBottom - jerseyTop};
        SDL_RenderFillRect(renderer, &jersey);
        
        // Jersey outline
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &jersey);
        
        // ===== BIG NUMBER on jersey =====
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        int numX = centerX;
        int numY = jerseyTop + 5;
        int numSize = 8;
        
        if (i < PLAYERS_PER_TEAM) {
            // Draw big "1"
            SDL_Rect num1 = {numX - 2, numY, 4, numSize};
            SDL_RenderFillRect(renderer, &num1);
            // Top cap
            SDL_RenderDrawLine(renderer, numX - 2, numY, numX - 4, numY + 2);
        } else {
            // Draw big "2"
            SDL_Rect top = {numX - 4, numY, 8, 2};
            SDL_Rect midRight = {numX + 2, numY, 2, 4};
            SDL_Rect mid = {numX - 4, numY + 3, 8, 2};
            SDL_Rect btmLeft = {numX - 4, numY + 3, 2, 5};
            SDL_Rect btm = {numX - 4, numY + 6, 8, 2};
            
            SDL_RenderFillRect(renderer, &top);
            SDL_RenderFillRect(renderer, &midRight);
            SDL_RenderFillRect(renderer, &mid);
            SDL_RenderFillRect(renderer, &btmLeft);
            SDL_RenderFillRect(renderer, &btm);
        }
        
        // ===== ARMS =====
        SDL_SetRenderDrawColor(renderer, players[i].r, players[i].g, players[i].b, 255);
        int armY = jerseyTop + 3;
        int armLength = 10;
        // Left arm (thick)
        SDL_RenderDrawLine(renderer, jerseyLeft, armY, jerseyLeft - armLength, armY + 5);
        SDL_RenderDrawLine(renderer, jerseyLeft, armY + 1, jerseyLeft - armLength, armY + 6);
        // Right arm (thick)
        SDL_RenderDrawLine(renderer, jerseyRight, armY, jerseyRight + armLength, armY + 5);
        SDL_RenderDrawLine(renderer, jerseyRight, armY + 1, jerseyRight + armLength, armY + 6);
        
        // Hands (skin color)
        SDL_SetRenderDrawColor(renderer, 255, 220, 177, 255);
        SDL_Rect leftHand = {jerseyLeft - armLength - 2, armY + 4, 3, 3};
        SDL_Rect rightHand = {jerseyRight + armLength - 1, armY + 4, 3, 3};
        SDL_RenderFillRect(renderer, &leftHand);
        SDL_RenderFillRect(renderer, &rightHand);
        
        // ===== SHORTS =====
        SDL_SetRenderDrawColor(renderer, players[i].r - 50, players[i].g - 50, players[i].b - 50, 255);
        SDL_Rect shorts = {jerseyLeft + 2, jerseyBottom, 16, 6};
        SDL_RenderFillRect(renderer, &shorts);
        
        // ===== LEGS =====
        int legTop = jerseyBottom + 6;
        int legBottom = players[i].y + players[i].h - 2;
        int legSpread = 4;
        
        // Skin color for legs
        SDL_SetRenderDrawColor(renderer, 255, 220, 177, 255);
        // Left leg (thick)
        SDL_Rect leftLeg = {centerX - legSpread - 2, legTop, 3, legBottom - legTop};
        SDL_RenderFillRect(renderer, &leftLeg);
        // Right leg (thick)
        SDL_Rect rightLeg = {centerX + legSpread - 1, legTop, 3, legBottom - legTop};
        SDL_RenderFillRect(renderer, &rightLeg);
        
        // ===== SHOES =====
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255); // Dark shoes
        SDL_Rect leftShoe = {centerX - legSpread - 3, legBottom, 5, 3};
        SDL_Rect rightShoe = {centerX + legSpread - 2, legBottom, 5, 3};
        SDL_RenderFillRect(renderer, &leftShoe);
        SDL_RenderFillRect(renderer, &rightShoe);
        
        // ===== ACTIVE PLAYER INDICATOR =====
        if (players[i].isActive) {
            // Draw yellow circle around active player
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            int indicatorRadius = 25;
            int indicatorY = players[i].y + players[i].h / 2;
            for (int angle = 0; angle < 360; angle += 8) {
                float rad1 = angle * 3.14159f / 180.0f;
                float rad2 = (angle + 8) * 3.14159f / 180.0f;
                int x1 = centerX + (int)(indicatorRadius * cos(rad1));
                int y1 = indicatorY + (int)(indicatorRadius * sin(rad1));
                int x2 = centerX + (int)(indicatorRadius * cos(rad2));
                int y2 = indicatorY + (int)(indicatorRadius * sin(rad2));
                SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
                SDL_RenderDrawLine(renderer, x1-1, y1, x2-1, y2); // Thicker
            }
        }
    }
}
// ===== TASK C: BALL IMPLEMENTATION =====

void Game::initBall() {
    ball.init(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);
}

void Game::updateBall(float deltaTime) {
    // Apply friction (simulation of air resistance)
    ball.vx *= ball.friction;
    ball.vy *= ball.friction;
    
    // Update position
    ball.x += ball.vx * deltaTime;
    ball.y += ball.vy * deltaTime;
    
    // Check wall collisions
    checkBallWallCollision(ball);

    //
    checkBallInCorner();
    
    // Check collision with all players
    for (int i = 0; i < TOTAL_PLAYERS; i++) {
        if (checkBallPlayerCollision(ball, players[i])) {
            // Ball hit a player
            std::cout << "[KICK] Player " << (i + 1) << " kicked the ball!\n";
        }
    }
    
    // Check if goal was scored
    int goalResult = checkGoal(ball);
    if (goalResult == 1) {
        // Team 2 scored to the left goal
        std::cout << "[GOAL] Team 2 (RED) scores! Total: " << score.team2Score + 1 << "\n";
        score.addGoal(1);
        // Reset players to starting positions and reset ball to center
        initPlayers();
        ball.reset(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);
    } else if (goalResult == 2) {
        // Team 1 scored to the right goal
        std::cout << "[GOAL] Team 1 (BLUE) scores! Total: " << score.team1Score + 1 << "\n";
        score.addGoal(0);
        // Reset players to starting positions and reset ball to center
        initPlayers();
        ball.reset(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);
    }
    
    // Stabilize very slow velocities
    float speedSq = ball.vx * ball.vx + ball.vy * ball.vy;
    if (speedSq < 1.0f) {
        ball.vx = 0.0f;
        ball.vy = 0.0f;
    }
}

void Game::renderBall() {
    // Draw ball as a white circle
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    
    int ballX = (int)ball.x;
    int ballY = (int)ball.y;
    int radius = (int)ball.radius;
    
    // Draw filled circle using midpoint circle algorithm
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x*x + y*y < radius*radius) {  // Use < instead of <= to avoid artifacts
                SDL_RenderDrawPoint(renderer, ballX + x, ballY + y);
            }
        }
    }
    
    // Draw ball outline
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    for (int angle = 0; angle < 360; angle += 4) {
        float rad1 = angle * 3.14159f / 180.0f;
        float rad2 = (angle + 4) * 3.14159f / 180.0f;
        int x1 = ballX + (int)(radius * cos(rad1));
        int y1 = ballY + (int)(radius * sin(rad1));
        int x2 = ballX + (int)(radius * cos(rad2));
        int y2 = ballY + (int)(radius * sin(rad2));
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }
}

void Game::renderScore() {
    if (!font) return;
    
    // Create score text
    std::string scoreText = "Team 1: " + std::to_string(score.team1Score) + 
                           "  vs  " + 
                           "Team 2: " + std::to_string(score.team2Score);
    
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Solid(font, scoreText.c_str(), white);
    
    if (!surface) return;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    
    // Center the score text at the top
    SDL_Rect scoreRect = {WINDOW_WIDTH / 2 - surface->w / 2, 20, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &scoreRect);
    
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}
// Simple AI for PVE mode (Team 1 - BLUE)
// (corner detection and nudging removed per user request)

// corner
Corner currentCorner = Corner::NONE;
void Game::checkBallInCorner() {
    float cornerSize = 30.0f;
        if (ball.x - ball.radius < FIELD_LEFT + cornerSize && ball.y - ball.radius < FIELD_TOP + cornerSize) {
            currentCorner = Corner::TOP_LEFT;
            std::cout << "[CORNER] Ball in TOP LEFT corner\n";
        } else if (ball.x + ball.radius > FIELD_RIGHT - cornerSize && ball.y - ball.radius < FIELD_TOP + cornerSize) {
            currentCorner = Corner::TOP_RIGHT;
            std::cout << "[CORNER] Ball in TOP RIGHT corner\n";
        } else if (ball.x - ball.radius < FIELD_LEFT + cornerSize && ball.y + ball.radius > FIELD_BOTTOM - cornerSize) {
            currentCorner = Corner::BOTTOM_LEFT;
            std::cout << "[CORNER] Ball in BOTTOM LEFT corner\n";
        } else if (ball.x + ball.radius > FIELD_RIGHT - cornerSize && ball.y + ball.radius > FIELD_BOTTOM - cornerSize) {
            currentCorner = Corner::BOTTOM_RIGHT;
            std::cout << "[CORNER] Ball in BOTTOM RIGHT corner\n";
        } else {
            currentCorner = Corner::NONE;
        }
}

void Game::updateAI(float deltaTime) {
    // Team 1 goals: left side
    float ownGoalX = FIELD_LEFT - 20.0f;
    float ownGoalY = WINDOW_HEIGHT / 2.0f;
    float fieldMidX = (FIELD_LEFT + FIELD_RIGHT) * 0.5f;

    // global AI speed multiplier (reduce to make AI slower)
    const float aiSpeedMult = 0.5f;

    // ----- Player 0: normally position between ball and own goal; if ball in AI half, dash to ball -----
    {
        int i = 0;
        float px = players[i].x + players[i].w / 2.0f;
        float py = players[i].y + players[i].h / 2.0f;

        // corner
        if (currentCorner != Corner::NONE && (std::sqrt((ball.x - px)*(ball.x - px) + (ball.y - py)*(ball.y - py)) < 50.0f)) {
            // Move towards center of field to escape corner
            float targetX = WINDOW_WIDTH / 2.0f;
            float targetY = WINDOW_HEIGHT / 2.0f;
            float dx = targetX - px;
            float dy = targetY - py;
            float dist = std::sqrt(dx*dx + dy*dy);
            if (dist > 1.0f) {
                players[i].x += (dx / dist) * players[i].speed * aiSpeedMult * deltaTime;
                players[i].y += (dy / dist) * players[i].speed * aiSpeedMult * deltaTime;
            }
        } else {
            if (ball.x < fieldMidX) {
                // Dash to ball aggressively but clamped to left half
                float dx = ball.x - px;
                float dy = ball.y - py;
                float dist = std::sqrt(dx*dx + dy*dy);
                float kickRange = 28.0f + ball.radius;
                if (dist > 1.0f) {
                    float dashSpeed = 1.4f * aiSpeedMult; // faster dash
                    players[i].x += (dx / dist) * players[i].speed * dashSpeed * deltaTime;
                    players[i].y += (dy / dist) * players[i].speed * dashSpeed * deltaTime;
                }
                // If within kicking range and ball is in AI half, kick it away from the player
                if (dist <= kickRange) {
                    // Only kick if ball is in front of player (to the right for Team 1)
                    // require the ball's left edge to be strictly in front of player's center
                    if (ball.x - ball.radius > px) {
                        // direction from player to ball
                        float nx = ball.x - px;
                        float ny = ball.y - py;
                        float nlen = std::sqrt(nx*nx + ny*ny);
                        if (nlen < 0.001f) nlen = 1.0f;
                        nx /= nlen; ny /= nlen;

                        float kickForce = 420.0f; // moderate kick force
                        std::cout << "[AI KICK] p0 center=" << px << " ball.center=" << ball.x << " ball.left=" << (ball.x - ball.radius) << " dist=" << dist << "\n";
                        ball.kick(nx, ny, kickForce);
                    }
                }
                // clamp to left half
                float margin = 20.0f;
                if (players[i].x + players[i].w/2.0f > fieldMidX - margin) {
                    players[i].x = fieldMidX - margin - players[i].w/2.0f;
                }
            } else {
                // default behavior: position between ball and own goal
                float gx = ownGoalX - ball.x;
                float gy = ownGoalY - ball.y;
                float glen = std::sqrt(gx*gx + gy*gy);
                if (glen < 0.0001f) glen = 1.0f;
                gx /= glen; gy /= glen;

                float offset = 80.0f; // distance from ball towards own goal
                float targetX = ball.x + gx * offset;
                float targetY = ball.y + gy * offset;

                if (targetX + players[i].w/2.0f > fieldMidX - 40.0f) {
                    targetX = fieldMidX - 40.0f - players[i].w/2.0f;
                }

                float dx = targetX - px;
                float dy = targetY - py;
                float dist = std::sqrt(dx*dx + dy*dy);
                if (dist > 1.0f) {
                    float speedFactor = 0.9f * aiSpeedMult;
                    players[i].x += (dx / dist) * players[i].speed * speedFactor * deltaTime;
                    players[i].y += (dy / dist) * players[i].speed * speedFactor * deltaTime;
                }
            }
        }
    }

    // ----- Players 1 and 2: dynamic chaser/supporter behavior -----
    {
        int a = 1, b = 2;
        float ax = players[a].x + players[a].w/2.0f;
        float ay = players[a].y + players[a].h/2.0f;
        float bx = players[b].x + players[b].w/2.0f;
        float by = players[b].y + players[b].h/2.0f;

        float distA = std::sqrt((ball.x - ax)*(ball.x - ax) + (ball.y - ay)*(ball.y - ay));
        float distB = std::sqrt((ball.x - bx)*(ball.x - bx) + (ball.y - by)*(ball.y - by));

        int chaser = (distA < distB) ? a : b;
        int supporter = (chaser == a) ? b : a;

        if (currentCorner != Corner::NONE && (distA < 50.0f || distB < 50.0f)) {
            // If ball is in corner, both players should try to move towards center of field to escape corner
            int i1 = chaser, i2 = supporter;
            float targetX = WINDOW_WIDTH / 2.0f;
            float targetY = WINDOW_HEIGHT / 2.0f;

            for (int i : {i1, i2}) {
                float px = players[i].x + players[i].w/2.0f;
                float py = players[i].y + players[i].h/2.0f;
                float dx = targetX - px;
                float dy = targetY - py;
                float dist = std::sqrt(dx*dx + dy*dy);
                if (dist > 1.0f) {
                    players[i].x += (dx / dist) * players[i].speed * aiSpeedMult * deltaTime;
                    players[i].y += (dy / dist) * players[i].speed * aiSpeedMult * deltaTime;
                }
            }
        }

        // --- Chaser: pursue ball and kick when in range ---
        {
            int i = chaser;
            float px = players[i].x + players[i].w/2.0f;
            float py = players[i].y + players[i].h/2.0f;
            float dx = ball.x - px;
            float dy = ball.y - py;
            float dist = std::sqrt(dx*dx + dy*dy);
            float kickRange = 28.0f + ball.radius;

            if (dist <= kickRange) {
                float pxCenter = players[i].x + players[i].w/2.0f;
                // require the ball's left edge to be in front of player's center
                if (ball.x - ball.radius > pxCenter) {
                    float oppGoalX = FIELD_RIGHT + 30.0f;
                    float oppGoalY = WINDOW_HEIGHT / 2.0f;
                    float gx = oppGoalX - ball.x;
                    float gy = oppGoalY - ball.y;
                    float glen = std::sqrt(gx*gx + gy*gy);
                    if (glen < 0.001f) glen = 1.0f;
                    gx /= glen; gy /= glen;
                    float kickForce = 380.0f;
                    std::cout << "[AI KICK] p" << i << " center=" << pxCenter << " ball.center=" << ball.x << "\n";
                    ball.kick(gx, gy, kickForce);
                } else {
                    if (dist > 0.5f) {
                        float speedFactor = aiSpeedMult;
                        players[i].x += (dx / dist) * players[i].speed * speedFactor * deltaTime;
                        players[i].y += (dy / dist) * players[i].speed * speedFactor * deltaTime;
                    }
                }
            } else {
                if (dist > 1.0f) {
                    float speedFactor = aiSpeedMult;
                    players[i].x += (dx / dist) * players[i].speed * speedFactor * deltaTime;
                    players[i].y += (dy / dist) * players[i].speed * speedFactor * deltaTime;
                }
            }
        }

        // --- Supporter: mirror chaser to create passing lane / provide support ---
        {
            int i = supporter;
            int lead = chaser;
            float leadX = players[lead].x + players[lead].w/2.0f;
            float leadY = players[lead].y + players[lead].h/2.0f;
            float centerY = WINDOW_HEIGHT / 2.0f;
            float mirroredY = 2.0f * centerY - leadY;
            float smallOffsetX = 20.0f;
            float targetX = leadX + smallOffsetX;
            float targetY = mirroredY;

            float px = players[i].x + players[i].w/2.0f;
            float py = players[i].y + players[i].h/2.0f;
            float dx = targetX - px;
            float dy = targetY - py;
            float dist = std::sqrt(dx*dx + dy*dy);

            float distToBall = std::sqrt((ball.x - px)*(ball.x - px) + (ball.y - py)*(ball.y - py));
            float kickRange = 28.0f + ball.radius;

            if (distToBall <= kickRange) {
                float pxCenter = players[i].x + players[i].w/2.0f;
                if (ball.x - ball.radius > pxCenter) {
                    float oppGoalX = FIELD_RIGHT + 30.0f;
                    float oppGoalY = WINDOW_HEIGHT / 2.0f;
                    float gx = oppGoalX - ball.x;
                    float gy = oppGoalY - ball.y;
                    float glen = std::sqrt(gx*gx + gy*gy);
                    if (glen < 0.001f) glen = 1.0f;
                    gx /= glen; gy /= glen;
                    float kickForce = 350.0f;
                    std::cout << "[AI KICK] p" << i << " center=" << pxCenter << " ball.center=" << ball.x << "\n";
                    ball.kick(gx, gy, kickForce);
                } else {
                    if (dist > 0.5f) {
                        float speedFactor = aiSpeedMult * 0.9f;
                        players[i].x += (dx / dist) * players[i].speed * speedFactor * deltaTime;
                        players[i].y += (dy / dist) * players[i].speed * speedFactor * deltaTime;
                    }
                }
            } else {
                if (dist > 1.0f) {
                    float speedFactor = aiSpeedMult * 0.9f;
                    players[i].x += (dx / dist) * players[i].speed * speedFactor * deltaTime;
                    players[i].y += (dy / dist) * players[i].speed * speedFactor * deltaTime;
                }
            }
        }
    }
}