// Implementations for Ball
#include "ball.h"
#include <cmath>

void Ball::init(float startX, float startY) {
    x = startX;
    y = startY;
    vx = 0.0f;
    vy = 0.0f;
    radius = 10.0f;
    speed = 0.0f;
    maxSpeed = 400.0f;   // pixels/sec max speed
    friction = 0.98f;    // Friction applied each frame
}

void Ball::getBounds(float& left, float& right, float& top, float& bottom) const {
    left = x - radius;
    right = x + radius;
    top = y - radius;
    bottom = y + radius;
}

void Ball::reset(float centerX, float centerY) {
    x = centerX;
    y = centerY;
    vx = 0.0f;
    vy = 0.0f;
}

void Ball::kick(float dirX, float dirY, float force) {
    float len = std::sqrt(dirX * dirX + dirY * dirY);
    if (len > 0.01f) {
        vx = (dirX / len) * force;
        vy = (dirY / len) * force;
    }
}
