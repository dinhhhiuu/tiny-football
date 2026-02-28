#include "utils.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cmath>
#include <string>

// render the start screen card and visuals
void renderStartScreen(SDL_Renderer* renderer, TTF_Font* fontSmall, TTF_Font* fontLarge, Player players[]) {
    renderFieldInStart(renderer);
    renderPlayerInStart(renderer, players);
    renderBallInStart(renderer);
    renderTextInStart(renderer, fontSmall, fontLarge);
}

void renderFieldInStart(SDL_Renderer* renderer) {
    // Background
    SDL_SetRenderDrawColor(renderer, COLOR_FIELD_R, COLOR_FIELD_G, COLOR_FIELD_B, 255);
    SDL_RenderClear(renderer);

    // ===== Center circle Half =====

    // const
    int centerRadius = 100;
    int centerX = 0;
    int centerY = WINDOW_HEIGHT / 2;

    int goalWidth = 140;
    int goalDepth = 40;

    int penaltyWidth = 220;   
    int penaltyDepth = 100; 

    int cornerSize = 10;

    int thickness = 3;

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // White lines
    
    for (int r = centerRadius - 2; r <= centerRadius + 2; r++) {
        for (int angle = 0; angle < 360; angle += 5) {
            float rad1 = angle * 3.14159f / 180.0f;
            float rad2 = (angle + 5) * 3.14159f / 180.0f;

            int x1 = centerX + (int)(r * cos(rad1));
            int y1 = centerY + (int)(r * sin(rad1));

            int x2 = centerX + (int)(r * cos(rad2));
            int y2 = centerY + (int)(r * sin(rad2));

            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        }
    }

    // ===== Right goal =====
    SDL_Rect rightGoal = { WINDOW_WIDTH - goalDepth, WINDOW_HEIGHT / 2 - goalWidth / 2, goalDepth, goalWidth };
    for (int i = 0; i < thickness; i++) {
        SDL_Rect g = {
            rightGoal.x - i,
            rightGoal.y - i,
            rightGoal.w + 2*i,
            rightGoal.h + 2*i
        };
        SDL_RenderDrawRect(renderer, &g);
    }
    
    // Right penalty area
    SDL_Rect rightPenalty = {
        WINDOW_WIDTH - goalDepth - penaltyDepth,
        WINDOW_HEIGHT / 2 - penaltyWidth/2,
        penaltyDepth,
        penaltyWidth
    };

    for (int i = 0; i < thickness; i++) {
        SDL_Rect p = {
            rightPenalty.x - i,
            rightPenalty.y - i,
            rightPenalty.w + 2*i,
            rightPenalty.h + 2*i
        };
        SDL_RenderDrawRect(renderer, &p);
    }
    
    // Right penalty spot
    SDL_Rect rightSpot = {WINDOW_WIDTH - goalDepth - penaltyDepth/2 - penaltyDepth/4, WINDOW_HEIGHT / 2, thickness * 2, thickness * 2};
    SDL_RenderFillRect(renderer, &rightSpot);

    // ===== Corner marks (thicker filled marks to ensure visibility) =====

    // Top-right
    SDL_Rect trH = { WINDOW_WIDTH - cornerSize - goalDepth, 0, cornerSize, thickness };
    SDL_Rect trV = { WINDOW_WIDTH - thickness - goalDepth, 0, thickness, cornerSize };
    SDL_RenderFillRect(renderer, &trH);
    SDL_RenderFillRect(renderer, &trV);

    // Bottom-right
    SDL_Rect brH = { WINDOW_WIDTH - cornerSize - goalDepth, WINDOW_HEIGHT - thickness, cornerSize, thickness };
    SDL_Rect brV = { WINDOW_WIDTH - thickness - goalDepth, WINDOW_HEIGHT - cornerSize, thickness, cornerSize };
    SDL_RenderFillRect(renderer, &brH);
    SDL_RenderFillRect(renderer, &brV);

    // Line
    SDL_Rect midLine = {
        WINDOW_WIDTH - goalDepth - thickness/2,
        0,
        thickness,
        WINDOW_HEIGHT
    };

    SDL_RenderFillRect(renderer, &midLine);
}

void renderPlayerInStart(SDL_Renderer* renderer, Player players[]) {
for (int i = 0; i < TOTAL_PLAYERS; i += 2) {
        int xPlayer = 300 + i * 60; 
        int yPlayer = WINDOW_HEIGHT / 2 + i * 10;
        int hPlayer = 80;

        if (i == 0) {
            xPlayer = 490; 
            yPlayer = 100;
        } else if (i == 2) {
            xPlayer = 400; 
            yPlayer = 180;
        } else if (i == 4) {
            xPlayer = 650; 
            yPlayer = 250;
        } 

        // Set player color
        SDL_SetRenderDrawColor(renderer, players[i].r, players[i].g, players[i].b, 255);
        
        // ===== HEAD (circle with face color) =====
        int headRadius = 20;
        SDL_SetRenderDrawColor(renderer, 255, 220, 177, 255); // Skin tone
        for (int y = -headRadius; y <= headRadius; y++) {
            for (int x = -headRadius; x <= headRadius; x++) {
                if (x*x + y*y <= headRadius*headRadius) {
                    SDL_RenderDrawPoint(renderer, xPlayer + x, yPlayer + y);
                }
            }
        }
        
        // Eyes
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        int EyeRadius = 3;
        for (int y = -EyeRadius; y <= EyeRadius; y++) {
            for (int x = -EyeRadius; x <= EyeRadius; x++) {
                if (x*x + y*y <= EyeRadius*EyeRadius) {
                    SDL_RenderDrawPoint(renderer, xPlayer - 6 + x, yPlayer - 5 + y); // Left eye
                    SDL_RenderDrawPoint(renderer, xPlayer + 6 + x, yPlayer - 5 + y); // Right eye
                }
            }
        }

        // ===== JERSEY (rectangle with number) =====
        int jerseyWidth = 60;
        int shortsHeight = 15;

        int jerseyTop = yPlayer + headRadius;
        int jerseyBottom = jerseyTop + hPlayer - shortsHeight; // 12: space for shorts
        int jerseyLeft = xPlayer - jerseyWidth/2;
        int jerseyRight = xPlayer + jerseyWidth/2;

        int jerseyHeight = jerseyBottom - jerseyTop;
        
        // Draw jersey body
        SDL_SetRenderDrawColor(renderer, players[i].r, players[i].g, players[i].b, 255);
        SDL_Rect jersey = {jerseyLeft, jerseyTop, jerseyWidth, jerseyHeight};
        SDL_RenderFillRect(renderer, &jersey);
        
        // Jersey outline
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &jersey);
        
        // ===== BIG NUMBER on jersey =====
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        int xCenterJersey = xPlayer;
        int yCenterJersey = jerseyTop + jerseyHeight / 2;
        int numSize = 20;

        if (i == 0 || i == 4) {
            // Draw big "1"
            SDL_Rect num1 = {xCenterJersey - 2, yCenterJersey - numSize/2, numSize/3, numSize + numSize/4};
            SDL_Rect num1_cross = {xCenterJersey - numSize/3, yCenterJersey - numSize/4, numSize/2, numSize/4};
            SDL_RenderFillRect(renderer, &num1_cross);
            SDL_RenderFillRect(renderer, &num1);
        } 
        else {
            // Draw big "2"
            SDL_Rect top = {xCenterJersey - numSize/2, yCenterJersey - numSize/2 - numSize/4,  numSize, numSize/4};
            SDL_Rect midRight = {xCenterJersey + numSize/2 - numSize/4, yCenterJersey - numSize/2, numSize/4, numSize - numSize/3}; 
            SDL_Rect mid = {xCenterJersey - numSize/2, yCenterJersey + numSize/4 - numSize/3, numSize, numSize/4};
            SDL_Rect btmLeft = {xCenterJersey - numSize/2, yCenterJersey, numSize/4, numSize - numSize/4}; 
            SDL_Rect btm = {xCenterJersey - numSize/2, yCenterJersey + numSize/2 + numSize/4, numSize, numSize/4};
            
            SDL_RenderFillRect(renderer, &top);
            SDL_RenderFillRect(renderer, &midRight);
            SDL_RenderFillRect(renderer, &mid);
            SDL_RenderFillRect(renderer, &btmLeft);
            SDL_RenderFillRect(renderer, &btm);
        }
        
        // ===== ARMS =====
        SDL_SetRenderDrawColor(renderer, players[i].r, players[i].g, players[i].b, 255);
        int armY = jerseyTop + 2; // 2: a little down from top of jersey to position arms better
        int armLength = 20;
        int thickness = 10;

        SDL_SetRenderDrawColor(renderer, 255, 220, 177, 255);
        // Hand left
        for (int t = 0; t < thickness; t++) {
            SDL_RenderDrawLine(
                renderer,
                jerseyLeft - 1,
                armY + t,
                jerseyLeft - armLength - 6,
                armY + t + 8
            );
        }

        // Hand right
        for (int t = 0; t < thickness; t++) {
            SDL_RenderDrawLine(
                renderer,
                jerseyRight + 1,
                armY + t,
                jerseyRight + armLength + 6,
                armY + t + 8
            );
        }

        SDL_SetRenderDrawColor(renderer, players[i].r, players[i].g, players[i].b, 255);
        // Right arm (thick)
        for (int t = 0; t <= thickness; t++) {
            SDL_RenderDrawLine(
                renderer,
                jerseyLeft - 1,
                armY + t,
                jerseyLeft - armLength,
                armY + t + 5
            );
        }
        // Right arm (thick)
        for (int t = 0; t <= thickness; t++) {
            SDL_RenderDrawLine(
                renderer,
                jerseyRight,
                armY + t,
                jerseyRight + armLength,
                armY + t + 5
            );
        }
        
        // ===== SHORTS =====
        SDL_SetRenderDrawColor(renderer, players[i].r - 50, players[i].g - 50, players[i].b - 50, 255);
        SDL_Rect shorts = {xPlayer - jerseyWidth/2, jerseyBottom, jerseyWidth, jerseyWidth/4};
        SDL_RenderFillRect(renderer, &shorts);
        
        // ===== LEGS =====
        int legTop = jerseyBottom + jerseyWidth/4;
        int legBottom = jerseyBottom + jerseyWidth/4 + shortsHeight;
        int legSpread = 8;
        
        // Skin color for legs
        SDL_SetRenderDrawColor(renderer, 255, 220, 177, 255);
        // Left leg (thick)
        SDL_Rect leftLeg = {xPlayer - jerseyWidth/2, legTop, jerseyWidth/2 - legSpread, legBottom - legTop};
        SDL_RenderFillRect(renderer, &leftLeg);
        // Right leg (thick)
        SDL_Rect rightLeg = {xPlayer + legSpread, legTop, jerseyWidth/2 - legSpread, legBottom - legTop};
        SDL_RenderFillRect(renderer, &rightLeg);
        
        // ===== SHOES =====
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255); // Dark shoes
        SDL_Rect leftShoe = {xPlayer - jerseyWidth/2, legBottom, jerseyWidth/2 - legSpread, 8};
        SDL_Rect rightShoe = {xPlayer + legSpread, legBottom, jerseyWidth/2 - legSpread, 8};
        SDL_RenderFillRect(renderer, &leftShoe);
        SDL_RenderFillRect(renderer, &rightShoe);
    }
}

void renderBallInStart(SDL_Renderer* renderer) {
    ////////////////////////////// ball
    // Draw ball as a white circle
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    
    int ballX = 540;
    int ballY = 260;
    int radius = 25;
    
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

void renderTextInStart(SDL_Renderer* renderer, TTF_Font* fontSmall, TTF_Font* fontLarge) {
    // color
    SDL_Color white = {255,255,255,255};
    SDL_Color black = {0,0,0,255};

    SDL_Surface* tinyWhite = TTF_RenderText_Solid(fontLarge, "TINY", white);
    SDL_Surface* tinyShadow = TTF_RenderText_Solid(fontLarge, "TINY", black);

    SDL_Surface* footWhite = TTF_RenderText_Solid(fontLarge, "FOOTBALL", white);
    SDL_Surface* footShadow = TTF_RenderText_Solid(fontLarge, "FOOTBALL", black);

    if (!tinyWhite || !tinyShadow || !footWhite || !footShadow) {
        if (tinyWhite) SDL_FreeSurface(tinyWhite);
        if (tinyShadow) SDL_FreeSurface(tinyShadow);
        if (footWhite) SDL_FreeSurface(footWhite);
        if (footShadow) SDL_FreeSurface(footShadow);
        return;
    }

    int gapBetweenLines = 6;
    int textHeight = tinyWhite->h + gapBetweenLines + footWhite->h;

    // ===== TITLE POSITION =====
    int titleX = 50;
    int titleY = WINDOW_HEIGHT / 2 - (textHeight) / 2;

    int shadowOffset = 3;

    // ================= TINY =================
    SDL_Texture* tinyShadowTex = SDL_CreateTextureFromSurface(renderer, tinyShadow);
    SDL_Rect tinyShadowRect = {
        titleX + (footWhite->w/2 - tinyWhite->w/2) + shadowOffset - 8,
        titleY + shadowOffset,
        tinyShadow->w,
        tinyShadow->h
    };
    SDL_RenderCopy(renderer, tinyShadowTex, nullptr, &tinyShadowRect);

    SDL_Texture* tinyTex = SDL_CreateTextureFromSurface(renderer, tinyWhite);
    SDL_Rect tinyRect = {
        titleX + (footWhite->w/2 - tinyWhite->w/2) - 8,
        titleY,
        tinyWhite->w,
        tinyWhite->h
    };
    SDL_RenderCopy(renderer, tinyTex, nullptr, &tinyRect);

    // cleanup tiny
    SDL_DestroyTexture(tinyShadowTex);
    SDL_DestroyTexture(tinyTex);
    SDL_FreeSurface(tinyShadow);
    SDL_FreeSurface(tinyWhite);

    titleY += tinyRect.h + gapBetweenLines;

    // ================= FOOTBALL =================
    SDL_Texture* footShadowTex = SDL_CreateTextureFromSurface(renderer, footShadow);
    SDL_Rect footShadowRect = {
        titleX + shadowOffset,
        titleY + shadowOffset,
        footShadow->w,
        footShadow->h
    };
    SDL_RenderCopy(renderer, footShadowTex, nullptr, &footShadowRect);

    SDL_Texture* footTex = SDL_CreateTextureFromSurface(renderer, footWhite);
    SDL_Rect footRect = {
        titleX,
        titleY,
        footWhite->w,
        footWhite->h
    };
    SDL_RenderCopy(renderer, footTex, nullptr, &footRect);

    // cleanup football
    SDL_DestroyTexture(footShadowTex);
    SDL_DestroyTexture(footTex);
    SDL_FreeSurface(footShadow);
    SDL_FreeSurface(footWhite);

    // Start / exit hints 
    SDL_Surface* enterSurface = TTF_RenderText_Solid(fontSmall, "ENTER = continue", white);
    if (enterSurface) {
        SDL_Texture* enterTex = SDL_CreateTextureFromSurface(renderer, enterSurface);
        SDL_Rect enterRect = { WINDOW_WIDTH/2 - enterSurface->w/2, WINDOW_HEIGHT/2 + 180, enterSurface->w, enterSurface->h };
        SDL_RenderCopy(renderer, enterTex, nullptr, &enterRect);
        SDL_DestroyTexture(enterTex);
        SDL_FreeSurface(enterSurface);
    }

    SDL_Surface* escSurface = TTF_RenderText_Solid(fontSmall, "ESC = Exit", white);
    if (escSurface) {
        SDL_Texture* escTex = SDL_CreateTextureFromSurface(renderer, escSurface);
        SDL_Rect escRect = { WINDOW_WIDTH/2 - escSurface->w/2, WINDOW_HEIGHT/2 + enterSurface->h + 160 + 40, escSurface->w, escSurface->h };
        SDL_RenderCopy(renderer, escTex, nullptr, &escRect);
        SDL_DestroyTexture(escTex);
        SDL_FreeSurface(escSurface);
    }
}

// Render the menu screen where players choose game mode
void renderMenuScreen(SDL_Renderer* renderer, TTF_Font* font, TTF_Font* fontSmall, TTF_Font* fontLarge, GameMode mode) {
    renderFieldInStart(renderer);

    // ===== Panel =====
    int panelW = 560;
    int panelH = 340;
    SDL_Rect panel = {
        WINDOW_WIDTH/2 - panelW/2,
        WINDOW_HEIGHT/2 - panelH/2,
        panelW,
        panelH
    };

    SDL_SetRenderDrawColor(renderer, 24, 28, 34, 255);
    SDL_RenderFillRect(renderer, &panel);

    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
    SDL_RenderDrawRect(renderer, &panel);

    if (!font) return;

    SDL_Color white = {255,255,255,255};
    SDL_Color highlight = {120,220,120,255};

    int padding = 20;
    int currentY = panel.y + padding;

    auto renderCenteredText = [&](const std::string& text, SDL_Color color, TTF_Font* fontToUse) {
        SDL_Surface* surface = TTF_RenderText_Solid(fontToUse, text.c_str(), color);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

        SDL_Rect rect = {
            panel.x + (panel.w - surface->w)/2, 
            currentY,
            surface->w,
            surface->h
        };

        SDL_RenderCopy(renderer, texture, nullptr, &rect);
        currentY += surface->h + 15;

        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    };

    // ===== Title =====
    renderCenteredText("TINY FOOTBALL", highlight, fontLarge);
    currentY += 10;

    // ===== Options =====
    SDL_Color sel1 = (mode == GameMode::PVP) ? highlight : white;
    SDL_Color sel2 = (mode == GameMode::PVE) ? highlight : white;

    renderCenteredText("1: PVP (Player vs Player)", sel1, font);
    renderCenteredText("2: PVE (Player vs AI)", sel2, font);

    currentY += 40;

    // ===== Hint =====
    renderCenteredText("Press number to choose mode", white, fontSmall);
    renderCenteredText("Press ENTER to start", white, fontSmall);
}

// Render the results screen after a match ends, showing final score and winner
void renderResultsScreen(SDL_Renderer* renderer, TTF_Font* font, TTF_Font* fontSmall, TTF_Font* fontLarge, Score score) {
    // Background field
    renderFieldInStart(renderer);

    // translucent overlay to focus panel
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
    SDL_Rect full = {0,0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_RenderFillRect(renderer, &full);

    // Panel
    const int panelW = 520;
    const int panelH = 300;
    SDL_Rect panel = { WINDOW_WIDTH/2 - panelW/2, WINDOW_HEIGHT/2 - panelH/2, panelW, panelH };
    SDL_SetRenderDrawColor(renderer, 28, 34, 44, 230);
    SDL_RenderFillRect(renderer, &panel);
    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 200);
    SDL_RenderDrawRect(renderer, &panel);

    if (!font && !fontLarge) return;
    TTF_Font* titleFont = fontLarge ? fontLarge : font;
    TTF_Font* bodyFont = font ? font : fontSmall;
    TTF_Font* hintFont = fontSmall ? fontSmall : font;

    SDL_Color white = {255,255,255,255};
    SDL_Color winColor = {150,230,150,255};
    SDL_Color loseColor = {230,150,150,255};
    SDL_Color drawColor = {200,200,200,255};

    // Title (large, centered)
    std::string title = "MATCH ENDED";
    SDL_Surface* sTitle = TTF_RenderText_Blended(titleFont, title.c_str(), white);
    if (sTitle) {
        SDL_Texture* tTitle = SDL_CreateTextureFromSurface(renderer, sTitle);
        SDL_Rect rTitle = { panel.x + (panel.w - sTitle->w)/2, panel.y + 18, sTitle->w, sTitle->h };
        SDL_RenderCopy(renderer, tTitle, nullptr, &rTitle);
        SDL_DestroyTexture(tTitle);
        SDL_FreeSurface(sTitle);
    }

    // Score (bigger, spaced)
    std::string left = "Team 1: " + std::to_string(score.team1Score);
    std::string right = "Team 2: " + std::to_string(score.team2Score);
    SDL_Surface* sLeft = TTF_RenderText_Blended(bodyFont, left.c_str(), white);
    SDL_Surface* sRight = TTF_RenderText_Blended(bodyFont, right.c_str(), white);
    if (sLeft && sRight) {
        // bring scores closer to center (tighten X) and slightly lower (closer to winner)
        int yScore = panel.y + 125;
        int centerX = panel.x + panel.w/2;
        int spacing = 8; // small gap between scores

        SDL_Texture* tLeft = SDL_CreateTextureFromSurface(renderer, sLeft);
        SDL_Texture* tRight = SDL_CreateTextureFromSurface(renderer, sRight);
        SDL_Rect rLeft = { centerX - spacing - sLeft->w, yScore, sLeft->w, sLeft->h };
        SDL_Rect rRight = { centerX + spacing, yScore, sRight->w, sRight->h };
        SDL_RenderCopy(renderer, tLeft, nullptr, &rLeft);
        SDL_RenderCopy(renderer, tRight, nullptr, &rRight);
        SDL_DestroyTexture(tLeft); SDL_DestroyTexture(tRight);
        SDL_FreeSurface(sLeft); SDL_FreeSurface(sRight);
    } else {
        if (sLeft) SDL_FreeSurface(sLeft);
        if (sRight) SDL_FreeSurface(sRight);
    }

    // Winner line
    std::string winner;
    SDL_Color winnerColor = white;
    if (score.team1Score > score.team2Score) { winner = "Team 1 (BLUE) wins!"; winnerColor = winColor; }
    else if (score.team2Score > score.team1Score) { winner = "Team 2 (RED) wins!"; winnerColor = loseColor; }
    else { winner = "Draw!"; winnerColor = drawColor; }

    SDL_Surface* sWinner = TTF_RenderText_Blended(bodyFont, winner.c_str(), winnerColor);
    if (sWinner) {
        SDL_Texture* tWinner = SDL_CreateTextureFromSurface(renderer, sWinner);
        // move winner a bit closer to the scores
        SDL_Rect rWin = { panel.x + (panel.w - sWinner->w)/2, panel.y + 155, sWinner->w, sWinner->h };
        SDL_RenderCopy(renderer, tWinner, nullptr, &rWin);
        SDL_DestroyTexture(tWinner);
        SDL_FreeSurface(sWinner);
    }

    // Hints (small)
    std::string hint1 = "Press ENTER to return to menu";
    std::string hint2 = "Press ESC to exit";
    SDL_Surface* sh1 = TTF_RenderText_Blended(hintFont, hint1.c_str(), white);
    SDL_Surface* sh2 = TTF_RenderText_Blended(hintFont, hint2.c_str(), white);
    if (sh1) {
        SDL_Texture* th1 = SDL_CreateTextureFromSurface(renderer, sh1);
        SDL_Rect rh1 = { panel.x + (panel.w - sh1->w)/2, panel.y + panel.h - 56, sh1->w, sh1->h };
        SDL_RenderCopy(renderer, th1, nullptr, &rh1);
        SDL_DestroyTexture(th1);
        SDL_FreeSurface(sh1);
    }
    if (sh2) {
        SDL_Texture* th2 = SDL_CreateTextureFromSurface(renderer, sh2);
        SDL_Rect rh2 = { panel.x + (panel.w - sh2->w)/2, panel.y + panel.h - 32, sh2->w, sh2->h };
        SDL_RenderCopy(renderer, th2, nullptr, &rh2);
        SDL_DestroyTexture(th2);
        SDL_FreeSurface(sh2);
    }

    // restore blend mode to default
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}