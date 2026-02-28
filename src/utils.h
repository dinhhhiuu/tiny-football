#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "config.h"
#include "state.h"
#include "score.h"

// Render the start screen card and visuals
void renderStartScreen(SDL_Renderer* renderer, TTF_Font* fontSmall, TTF_Font* fontLarge, Player players[]);

// util Start
void renderFieldInStart(SDL_Renderer* renderer);
void renderPlayerInStart(SDL_Renderer* renderer, Player players[]);
void renderBallInStart(SDL_Renderer* renderer);
void renderTextInStart(SDL_Renderer* renderer, TTF_Font* fontSmall, TTF_Font* fontLarge);

// Render the menu screen card and visuals
void renderMenuScreen(SDL_Renderer* renderer, TTF_Font* font, TTF_Font* fontSmall, TTF_Font* fontLarge, GameMode mode);

// Render the results screen card and visuals
void renderResultsScreen(SDL_Renderer* renderer, TTF_Font* font, TTF_Font* fontSmall, TTF_Font* fontLarge, Score score);