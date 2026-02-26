#pragma once

// Ball object for Tiny Football game
struct Ball {
    float x;             // Center position X
    float y;             // Center position Y
    float vx;            // Velocity X
    float vy;            // Velocity Y
    float radius;        // Ball radius
    float speed;         // Current speed magnitude
    float maxSpeed;      // Maximum speed
    float friction;      // Friction coefficient

    // Initialize the ball
    void init(float startX, float startY);

    // Get ball bounds for collision detection
    void getBounds(float& left, float& right, float& top, float& bottom) const;

    // Reset ball to center with no velocity
    void reset(float centerX, float centerY);

    // Kick the ball (apply velocity)
    void kick(float dirX, float dirY, float force);
};
