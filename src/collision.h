#pragma once
#include "ball.h"
#include "config.h"

// ===== BALL VS SCREEN EDGES =====
// Returns true if collision occurred and handles reflection
bool checkBallWallCollision(Ball& ball);

// ===== BALL VS PLAYER COLLISION =====
// Detects circular ball vs rectangular player collision
// Returns true if collision occurred
bool checkBallPlayerCollision(Ball& ball, const Player& player);

// ===== GOAL DETECTION =====
// Checks if ball is in goal zone and returns which team scored
// Returns: 0 = no goal, 1 = Team 2 scored (ball went to left goal), 2 = Team 1 scored (right goal)
int checkGoal(const Ball& ball);
