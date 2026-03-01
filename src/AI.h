#pragma once

#include "state.h"

// Expose the current corner state used by AI logic
extern Corner currentCorner;

void updateExtendedAI(float deltaTime, Player players[], Ball& ball);
void updateExtendedTeam2AI(float deltaTime, Player players[], Ball& ball, int activePlayerTeam2);