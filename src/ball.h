#pragma once
#include <cmath>

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
    
    // Constructor-like initializer
    void init(float startX, float startY) {
        x = startX;
        y = startY;
        vx = 0.0f;
        vy = 0.0f;
        radius = 10.0f;
        speed = 0.0f;
        maxSpeed = 400.0f;   // pixels/sec max speed
        friction = 0.98f;    // Friction applied each frame
    }
    
    // Get ball bounds for collision detection
    void getBounds(float& left, float& right, float& top, float& bottom) const {
        left = x - radius;
        right = x + radius;
        top = y - radius;
        bottom = y + radius;
    }
    
    // Reset ball to center with no velocity
    void reset(float centerX, float centerY) {
        x = centerX;
        y = centerY;
        vx = 0.0f;
        vy = 0.0f;
    }
    
    // Kick the ball (apply velocity)
    void kick(float dirX, float dirY, float force) {
        // Normalize direction and apply force
        float len = sqrtf(dirX * dirX + dirY * dirY);
        if (len > 0.01f) {
            vx = (dirX / len) * force;
            vy = (dirY / len) * force;
        }
    }
};
