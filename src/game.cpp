#include "game.h"
#include "utils.h"
#include "AI.h"

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <string>

Game::Game()
    : window(nullptr),
      renderer(nullptr),
      running(true),
      state(GameState::START),
      mode(GameMode::PVP),
      lastTick(0),
      matchTimeLeft(MATCH_TIME_SECONDS),
            // startBallTexture(nullptr),
            activePlayerTeam1(0),  // Start with first player of Team 1
            activePlayerTeam2(3),  // Start with first player of Team 2
            keyW(false), keyA(false), keyS(false), keyD(false),
            keyUp(false), keyDown(false), keyLeft(false), keyRight(false),
            windX(0.0f), windY(0.0f), windChangeTimer(0.0f), windDuration(5.0f) {}

Game::~Game() {}

bool Game::init() {
    std::cout << "Initializing SDL...\n";
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
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
        fontSmall = nullptr;
        fontLarge = nullptr;
    } else {
        font = TTF_OpenFont("assets/fonts/ZeroCool.ttf", 24);
        fontSmall = TTF_OpenFont("assets/fonts/ZeroCool.ttf", 16);
        fontLarge = TTF_OpenFont("assets/fonts/ZeroCool.ttf", 64);
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
    
    // Initialize wind system
    initWind();
    
    // Initialize sound system
    soundSystem.init();

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
        // if (e.type == SDL_KEYDOWN) {
        //     std::cout << "[EVENT] Key pressed: " << SDL_GetKeyName(e.key.keysym.sym) << "\n";
        // }
        
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
                    
                    // Play whistle sound to start the game
                    soundSystem.playWhistleSound();
                }
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    state = GameState::EXIT;
                }
            }
            else if (state == GameState::PLAY) {
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    state = GameState::START;
                }
                else if (e.key.keysym.sym == SDLK_p) {
                    state = GameState::PAUSE;
                    std::cout << "[PAUSE] Game paused\n";
                }
            }
            else if (state == GameState::PAUSE) {
                if (e.key.keysym.sym == SDLK_p || e.key.keysym.sym == SDLK_RETURN) {
                    state = GameState::PLAY;
                    std::cout << "[PAUSE] Game resumed\n";
                }
                else if (e.key.keysym.sym == SDLK_ESCAPE) {
                    state = GameState::START;
                    std::cout << "[PAUSE] Returning to menu\n";
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
        
        // Update wind
        updateWind(deltaTime);
        
        // Update particle effects
        particleSystem.update(deltaTime);
        goalAnimation.update(deltaTime);
        
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
    else if (state == GameState::PAUSE) {
        renderPause();
    }
    else if (state == GameState::RESULTS) {
        renderResults();
    }

    SDL_RenderPresent(renderer);
}

void Game::renderStart() {
    renderStartScreen(renderer, fontSmall, fontLarge, players);
}

void Game::renderMenu() {
    renderMenuScreen(renderer, font, fontSmall, fontLarge, mode);
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
    
    // Render wind indicator
    renderWindIndicator();
    
    // Render particle effects
    particleSystem.render(renderer);
    
    // Render goal animation on top
    goalAnimation.render(renderer, fontLarge);
    
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
    renderResultsScreen(renderer, font, fontSmall, fontLarge, score); 
}

void Game::renderPause() {
    // First render the paused game state in the background
    renderPlay();
    
    // Draw semi-transparent overlay
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180); // Dark overlay
    SDL_Rect overlay = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_RenderFillRect(renderer, &overlay);
    
    if (!font || !fontLarge) return;
    
    // Draw "PAUSED" text
    SDL_Color yellow = {255, 255, 0, 255};
    SDL_Surface* pauseSurf = TTF_RenderText_Solid(fontLarge, "PAUSED", yellow);
    if (pauseSurf) {
        SDL_Texture* pauseTex = SDL_CreateTextureFromSurface(renderer, pauseSurf);
        SDL_Rect pauseRect = {
            WINDOW_WIDTH/2 - pauseSurf->w/2,
            WINDOW_HEIGHT/2 - 80,
            pauseSurf->w,
            pauseSurf->h
        };
        SDL_RenderCopy(renderer, pauseTex, nullptr, &pauseRect);
        SDL_FreeSurface(pauseSurf);
        SDL_DestroyTexture(pauseTex);
    }
    
    // Draw instructions
    SDL_Color white = {255, 255, 255, 255};
    const char* instructions[] = {
        "Press P or ENTER to Resume",
        "Press ESC to Main Menu"
    };
    
    int yPos = WINDOW_HEIGHT/2 + 20;
    for (int i = 0; i < 2; i++) {
        SDL_Surface* surf = TTF_RenderText_Solid(font, instructions[i], white);
        if (surf) {
            SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_Rect rect = {
                WINDOW_WIDTH/2 - surf->w/2,
                yPos + i * 40,
                surf->w,
                surf->h
            };
            SDL_RenderCopy(renderer, tex, nullptr, &rect);
            SDL_FreeSurface(surf);
            SDL_DestroyTexture(tex);
        }
    }
}

void Game::clean() {
    soundSystem.cleanup();
    
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
        updateAI(deltaTime, players, ball);
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

    // If in PVE mode, let the non-active Team 2 players be controlled by AI
    if (mode == GameMode::PVE) {
        updateTeam2AI(deltaTime, players, ball, activePlayerTeam2);
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
    
    // Apply gentle wind force to ball
    float windForce = 30.0f; // Gentle wind strength
    ball.vx += windX * windForce * deltaTime;
    ball.vy += windY * windForce * deltaTime;
    
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
            soundSystem.playKickSound();
            
            // Create kick particle effect
            particleSystem.createKickEffect(ball.x, ball.y, ball.vx, ball.vy);
        }
    }
        
    // Check if goal was scored
    int goalResult = checkGoal(ball);
    if (goalResult == 1) {
        // Team 2 scored to the left goal
        std::cout << "[GOAL] Team 2 (RED) scores! Total: " << score.team2Score + 1 << "\n";
        soundSystem.playGoalSound();
        
        // Goal celebration effects
        particleSystem.createGoalExplosion(ball.x, ball.y);
        goalAnimation.start(1);
        
        score.addGoal(1);
        // Reset players to starting positions and reset ball to center
        initPlayers();
        ball.reset(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);
    } else if (goalResult == 2) {
        // Team 1 scored to the right goal
        std::cout << "[GOAL] Team 1 (BLUE) scores! Total: " << score.team1Score + 1 << "\n";
        soundSystem.playGoalSound();
        
        // Goal celebration effects
        particleSystem.createGoalExplosion(ball.x, ball.y);
        goalAnimation.start(0);
        
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

// (corner detection and nudging removed per user request)
extern Corner currentCorner;
void Game::checkBallInCorner() {
    float cornerSize = 20.0f;
        if (ball.x - ball.radius < FIELD_LEFT + cornerSize && ball.y - ball.radius < FIELD_TOP + cornerSize) {
            currentCorner = Corner::TOP_LEFT;
            // std::cout << "[CORNER] Ball in TOP LEFT corner\n";
        } else if (ball.x + ball.radius > FIELD_RIGHT - cornerSize && ball.y - ball.radius < FIELD_TOP + cornerSize) {
            currentCorner = Corner::TOP_RIGHT;
            // std::cout << "[CORNER] Ball in TOP RIGHT corner\n";
        } else if (ball.x - ball.radius < FIELD_LEFT + cornerSize && ball.y + ball.radius > FIELD_BOTTOM - cornerSize) {
            currentCorner = Corner::BOTTOM_LEFT;
            // std::cout << "[CORNER] Ball in BOTTOM LEFT corner\n";
        } else if (ball.x + ball.radius > FIELD_RIGHT - cornerSize && ball.y + ball.radius > FIELD_BOTTOM - cornerSize) {
            currentCorner = Corner::BOTTOM_RIGHT;
            // std::cout << "[CORNER] Ball in BOTTOM RIGHT corner\n";
        } else {
            currentCorner = Corner::NONE;
        }
}

// (AI implementations moved to src/AI.cpp)
void Game::updateAI(float deltaTime, Player players[], Ball& ball) {
    updateExtendedAI(deltaTime, players, ball);
}

void Game::updateTeam2AI(float deltaTime, Player players[], Ball& ball, int activePlayerIndex) {
    updateExtendedTeam2AI(deltaTime, players, ball, activePlayerIndex);
}

// ===== Wind system implementation =====
void Game::initWind() {
    // Start with gentle wind
    windX = 0.0f;
    windY = 0.0f;
    windChangeTimer = 3.0f; // First wind change after 3 seconds
}

void Game::updateWind(float deltaTime) {
    windChangeTimer -= deltaTime;
    
    if (windChangeTimer <= 0.0f) {
        // Change wind direction and strength randomly (gentler)
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float strength = 0.1f + (rand() % 50) / 100.0f; // 0.1 to 0.6 (gentle)
        
        windX = cos(angle) * strength;
        windY = sin(angle) * strength;
        
        // Reset timer (wind changes every 5-8 seconds)
        windChangeTimer = windDuration + (rand() % 3);
        
        // std::cout << "[WIND] Wind changed: direction=" << (int)(angle * 180.0f / 3.14159f) 
        //           << "° strength=" << strength << "\n";
    }
}

void Game::renderWindIndicator() {
    if (!font || !fontSmall) return;
    
    // Position at top-right, ABOVE the field
    int boxX = WINDOW_WIDTH - 95;
    int boxY = 5;
    int boxWidth = 85;
    int boxHeight = 40;
    
    // Draw background (brown/orange-tan like image)
    SDL_Rect bgRect = {boxX, boxY, boxWidth, boxHeight};
    SDL_SetRenderDrawColor(renderer, 90, 60, 40, 240);  // Darker brownish
    SDL_RenderFillRect(renderer, &bgRect);
    
    // Draw border (lighter brown/tan)
    SDL_SetRenderDrawColor(renderer, 160, 120, 80, 255);
    SDL_RenderDrawRect(renderer, &bgRect);
    
    // Draw "Wind" label at top (orange-red like image)
    SDL_Color labelColor = {255, 150, 80, 255};  // Orange-red
    SDL_Surface* labelSurface = TTF_RenderText_Solid(fontSmall, "Wind", labelColor);
    if (labelSurface) {
        SDL_Texture* labelTexture = SDL_CreateTextureFromSurface(renderer, labelSurface);
        SDL_Rect labelRect = {boxX + 3, boxY + 2, labelSurface->w, labelSurface->h};
        SDL_RenderCopy(renderer, labelTexture, nullptr, &labelRect);
        SDL_FreeSurface(labelSurface);
        SDL_DestroyTexture(labelTexture);
    }
    
    // Calculate wind components
    float windStrength = std::sqrt(windX * windX + windY * windY);
    int windValue1 = (int)(fabs(windX) * 10.0f);  // Horizontal component
    int windValue2 = (int)(fabs(windY) * 10.0f);  // Vertical component
    
    // Draw first number (green, left side)
    char text1[8];
    snprintf(text1, sizeof(text1), "%d", windValue1);
    SDL_Color greenColor = {100, 255, 100, 255};
    
    SDL_Surface* surf1 = TTF_RenderText_Solid(font, text1, greenColor);
    if (surf1) {
        SDL_Texture* tex1 = SDL_CreateTextureFromSurface(renderer, surf1);
        SDL_Rect rect1 = {boxX + 8, boxY + 18, surf1->w, surf1->h};
        SDL_RenderCopy(renderer, tex1, nullptr, &rect1);
        SDL_FreeSurface(surf1);
        SDL_DestroyTexture(tex1);
    }
    
    // Draw dash "-" (green)
    SDL_Surface* surfDash = TTF_RenderText_Solid(font, "-", greenColor);
    if (surfDash) {
        SDL_Texture* texDash = SDL_CreateTextureFromSurface(renderer, surfDash);
        SDL_Rect rectDash = {boxX + 25, boxY + 18, surfDash->w, surfDash->h};
        SDL_RenderCopy(renderer, texDash, nullptr, &rectDash);
        SDL_FreeSurface(surfDash);
        SDL_DestroyTexture(texDash);
    }
    
    // Draw second number (purple/blue, right side)
    char text2[8];
    snprintf(text2, sizeof(text2), "%d", windValue2);
    SDL_Color purpleColor = {150, 100, 255, 255};
    
    SDL_Surface* surf2 = TTF_RenderText_Solid(font, text2, purpleColor);
    if (surf2) {
        SDL_Texture* tex2 = SDL_CreateTextureFromSurface(renderer, surf2);
        SDL_Rect rect2 = {boxX + 35, boxY + 18, surf2->w, surf2->h};
        SDL_RenderCopy(renderer, tex2, nullptr, &rect2);
        SDL_FreeSurface(surf2);
        SDL_DestroyTexture(tex2);
    }
}