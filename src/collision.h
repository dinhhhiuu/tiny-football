#pragma once
#include <cmath>
#include "ball.h"
#include "config.h"

// ===== BALL VS SCREEN EDGES =====
// Returns true if collision occurred and handles reflection
inline bool checkBallWallCollision(Ball& ball) {
    bool collided = false;
    
    // Top wall
    if (ball.y - ball.radius < FIELD_TOP) {
        ball.y = FIELD_TOP + ball.radius;
        ball.vy = -ball.vy * 0.9f;  // Reflect with slight energy loss
        collided = true;
    }
    
    // Bottom wall
    if (ball.y + ball.radius > FIELD_BOTTOM) {
        ball.y = FIELD_BOTTOM - ball.radius;
        ball.vy = -ball.vy * 0.9f;
        collided = true;
    }
    
    // Left side wall (goal area)
    if (ball.x - ball.radius < FIELD_LEFT && (ball.y < WINDOW_HEIGHT/2 - 50 || ball.y > WINDOW_HEIGHT/2 + 50)) {
        ball.x = FIELD_LEFT + ball.radius;
        ball.vx = -ball.vx * 0.9f;
        collided = true;
    }
    
    // Right side wall (goal area)
    if (ball.x + ball.radius > FIELD_RIGHT && (ball.y < WINDOW_HEIGHT/2 - 50 || ball.y > WINDOW_HEIGHT/2 + 50)) {
        ball.x = FIELD_RIGHT - ball.radius;
        ball.vx = -ball.vx * 0.9f;
        collided = true;
    }
    
    return collided;
}

// ===== BALL VS PLAYER COLLISION =====
// Detects circular ball vs rectangular player collision
// Returns true if collision occurred
inline bool checkBallPlayerCollision(Ball& ball, const Player& player) {
    // Get ball bounds
    float ballLeft = ball.x - ball.radius;
    float ballRight = ball.x + ball.radius;
    float ballTop = ball.y - ball.radius;
    float ballBottom = ball.y + ball.radius;
    
    // Player bounds
    float playerLeft = player.x;
    float playerRight = player.x + player.w;
    float playerTop = player.y;
    float playerBottom = player.y + player.h;
    
    // Simple AABB collision first
    if (ballRight < playerLeft || ballLeft > playerRight ||
        ballBottom < playerTop || ballTop > playerBottom) {
        return false;  // No collision
    }
    
    // Ball hit the player - calculate bounce direction
    float playerCenterX = player.x + player.w / 2.0f;
    float playerCenterY = player.y + player.h / 2.0f;
    
    // Direction from player to ball
    float dirX = ball.x - playerCenterX;
    float dirY = ball.y - playerCenterY;
    float distLen = sqrtf(dirX * dirX + dirY * dirY);
    
    if (distLen > 0.01f) {
        // Normalize direction
        dirX /= distLen;
        dirY /= distLen;
        
        // Push ball out to prevent overlap
        float overlap = ball.radius + (player.w + player.h) / 4.0f;
        ball.x = playerCenterX + dirX * overlap;
        ball.y = playerCenterY + dirY * overlap;
    }
    
    // Apply bounce with kick force
    float kickForce = 300.0f;  // pixels/sec
    ball.vx = dirX * kickForce;
    ball.vy = dirY * kickForce;
    
    return true;
}

// ===== GOAL DETECTION =====
// Checks if ball is in goal zone and returns which team scored
// Returns: 0 = no goal, 1 = Team 2 scored (ball went to left goal), 2 = Team 1 scored (right goal)
inline int checkGoal(const Ball& ball) {
    int centerY = WINDOW_HEIGHT / 2;
    int goalWidth = 80;  // Goal height
    int goalTop = centerY - goalWidth / 2;
    int goalBottom = centerY + goalWidth / 2;
    
    // Ball in left goal zone
    if (ball.x - ball.radius < FIELD_LEFT - 20) {
        if (ball.y > goalTop && ball.y < goalBottom) {
            return 1;  // Team 2 scores (left goal)
        }
    }
    
    // Ball in right goal zone
    if (ball.x + ball.radius > FIELD_RIGHT + 20) {
        if (ball.y > goalTop && ball.y < goalBottom) {
            return 2;  // Team 1 scores (right goal)
        }
    }
    
    return 0;  // No goal
}
