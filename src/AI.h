#pragma once

#include "state.h"
#include "config.h"
#include "ball.h"

// static int minimumAvoidCorner = 0;
static int minimumAvoidBoundary[3] = {0, 0, 0};

// Expose the current corner state used by AI logic
extern Corner currentCorner;

// randomized tweak used by AI; defined in AI.cpp
extern float randomOffset;

void updateExtendedAI(float deltaTime, Player players[], Ball& ball);
void updateExtendedTeam2AI(float deltaTime, Player players[], Ball& ball, int activePlayerTeam2);
void applyFieldBoundary(Player& p, float deltaTime, Ball& ball);