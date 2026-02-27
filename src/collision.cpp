// Collision implementations
#include "collision.h"
#include <cmath>
#include <algorithm>

bool checkBallWallCollision(Ball& ball) {
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
    {
        int centerY = WINDOW_HEIGHT / 2;
        int goalWidth = 80; // must match renderField
        int goalDepth = 20;
        int goalTop = centerY - goalWidth/2;
        int goalBottom = centerY + goalWidth/2;

        if (ball.x - ball.radius < FIELD_LEFT) {
            // If ball is NOT within the vertical bounds of the goal, reflect it
            if (ball.y < goalTop || ball.y > goalBottom) {
                ball.x = FIELD_LEFT + ball.radius;
                ball.vx = -ball.vx * 0.9f;
                collided = true;
            } else {
                // Ball is within the goal mouth vertically. If it has passed the goal depth, allow goal detection elsewhere.
                // If it is only slightly past FIELD_LEFT but hasn't reached the goal depth, leave it alone to continue moving.
                if (ball.x - ball.radius < FIELD_LEFT - goalDepth) {
                    // let checkGoal handle scoring
                }
            }
        }
    }

    // Right side wall (goal area)
    {
        int centerY = WINDOW_HEIGHT / 2;
        int goalWidth = 80; // must match renderField
        int goalDepth = 20;
        int goalTop = centerY - goalWidth/2;
        int goalBottom = centerY + goalWidth/2;

        if (ball.x + ball.radius > FIELD_RIGHT) {
            if (ball.y < goalTop || ball.y > goalBottom) {
                ball.x = FIELD_RIGHT - ball.radius;
                ball.vx = -ball.vx * 0.9f;
                collided = true;
            } else {
                if (ball.x + ball.radius > FIELD_RIGHT + goalDepth) {
                    // let checkGoal detect score
                }
            }
        }
    }

    return collided;
}

bool checkBallPlayerCollision(Ball& ball, const Player& player) {
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

    // Ball hit the player - use physics-based response
    float playerCenterX = player.x + player.w / 2.0f;
    float playerCenterY = player.y + player.h / 2.0f;

    // Approximate player as circle for collision response
    float playerRadius = std::max(player.w, player.h) * 0.5f;

    // Direction from player to ball
    float nx = ball.x - playerCenterX;
    float ny = ball.y - playerCenterY;
    float dist = std::sqrt(nx * nx + ny * ny);
    if (dist < 0.0001f) {
        // Prevent divide by zero
        nx = 1.0f; ny = 0.0f; dist = 1.0f;
    }
    nx /= dist; ny /= dist; // normalize

    // Penetration depth
    float penetration = ball.radius + playerRadius - dist;
    if (penetration > 0.0f) {
        // Push ball out along normal
        ball.x += nx * penetration;
        ball.y += ny * penetration;
    }

    // Relative velocity (ball relative to player)
    float rvx = ball.vx - player.vx;
    float rvy = ball.vy - player.vy;

    // Velocity along normal
    float velAlongNormal = rvx * nx + rvy * ny;

    // Only resolve if moving towards each other
    if (velAlongNormal < 0.0f) {
        // Use unified bounce parameters for all players
        float e = 0.8f; // restitution (bounciness)
        float impulseScale = 2.5f; // impulse multiplier

        // Compute impulse directly from relative velocity and restitution
        float j = -(1.0f + e) * velAlongNormal * impulseScale;

        // Apply impulse to ball velocity
        ball.vx += j * nx;
        ball.vy += j * ny;
    }

    // Transfer a small portion of player's movement velocity to the ball (same for all players)
    float playerSpeed = std::sqrt(player.vx * player.vx + player.vy * player.vy);
    if (playerSpeed > 20.0f) {
        float influence = 0.12f;
        ball.vx += player.vx * influence;
        ball.vy += player.vy * influence;
    }

    // Clamp ball speed to reasonable max
    float speedSq = ball.vx * ball.vx + ball.vy * ball.vy;
    if (speedSq > ball.maxSpeed * ball.maxSpeed) {
        float scale = ball.maxSpeed / std::sqrt(speedSq);
        ball.vx *= scale;
        ball.vy *= scale;
    }

    return true;
}

int checkGoal(const Ball& ball) {
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
