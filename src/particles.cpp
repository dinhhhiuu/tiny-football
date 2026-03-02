#include "particles.h"
#include "config.h"
#include <SDL2/SDL_ttf.h>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <iostream>

// ===== Particle System =====

ParticleSystem::ParticleSystem() {
    particles.reserve(MAX_PARTICLES);
}

void ParticleSystem::createKickEffect(float x, float y, float ballVx, float ballVy) {
    // Create burst of particles in opposite direction of ball movement
    int numParticles = 20;
    
    for (int i = 0; i < numParticles; i++) {
        if (particles.size() >= MAX_PARTICLES) break;
        
        Particle p;
        p.x = x;
        p.y = y;
        
        // Random angle around opposite direction
        float baseAngle = atan2(-ballVy, -ballVx);
        float spread = 0.8f;
        float angle = baseAngle + ((rand() % 100 - 50) / 100.0f) * spread;
        
        float speed = 50.0f + (rand() % 100);
        p.vx = cos(angle) * speed;
        p.vy = sin(angle) * speed;
        
        p.lifetime = 0.0f;
        p.maxLifetime = 0.3f + (rand() % 30) / 100.0f;
        
        // White to yellow particles
        p.r = 255;
        p.g = 200 + rand() % 55;
        p.b = 100 + rand() % 100;
        
        p.size = 2.0f + (rand() % 3);
        
        particles.push_back(p);
    }
}

void ParticleSystem::createGoalExplosion(float x, float y) {
    // Create colorful explosion
    int numParticles = 80;
    
    for (int i = 0; i < numParticles; i++) {
        if (particles.size() >= MAX_PARTICLES) break;
        
        Particle p;
        p.x = x;
        p.y = y;
        
        float angle = (rand() % 360) * M_PI / 180.0f;
        float speed = 100.0f + (rand() % 200);
        p.vx = cos(angle) * speed;
        p.vy = sin(angle) * speed;
        
        p.lifetime = 0.0f;
        p.maxLifetime = 0.5f + (rand() % 80) / 100.0f;
        
        // Random bright colors
        int colorType = rand() % 5;
        switch(colorType) {
            case 0: p.r = 255; p.g = 50; p.b = 50; break;    // Red
            case 1: p.r = 50; p.g = 255; p.b = 50; break;    // Green
            case 2: p.r = 50; p.g = 50; p.b = 255; break;    // Blue
            case 3: p.r = 255; p.g = 255; p.b = 50; break;   // Yellow
            case 4: p.r = 255; p.g = 50; p.b = 255; break;   // Magenta
        }
        
        p.size = 3.0f + (rand() % 4);
        
        particles.push_back(p);
    }
}

void ParticleSystem::update(float deltaTime) {
    // Update all particles
    for (auto it = particles.begin(); it != particles.end();) {
        it->lifetime += deltaTime;
        
        if (it->lifetime >= it->maxLifetime) {
            it = particles.erase(it);
            continue;
        }
        
        // Apply gravity
        it->vy += 200.0f * deltaTime;
        
        // Update position
        it->x += it->vx * deltaTime;
        it->y += it->vy * deltaTime;
        
        ++it;
    }
}

void ParticleSystem::render(SDL_Renderer* renderer) {
    for (const auto& p : particles) {
        float lifeRatio = p.lifetime / p.maxLifetime;
        Uint8 alpha = (Uint8)(255 * (1.0f - lifeRatio));
        
        SDL_SetRenderDrawColor(renderer, p.r, p.g, p.b, alpha);
        
        // Draw particle as small filled circle
        int size = (int)(p.size * (1.0f - lifeRatio * 0.5f));
        for (int dy = -size; dy <= size; dy++) {
            for (int dx = -size; dx <= size; dx++) {
                if (dx*dx + dy*dy <= size*size) {
                    SDL_RenderDrawPoint(renderer, (int)p.x + dx, (int)p.y + dy);
                }
            }
        }
    }
}

void ParticleSystem::clear() {
    particles.clear();
}

// ===== Goal Animation =====

GoalAnimation::GoalAnimation() 
    : active(false), timer(0.0f), scoringTeam(0), scale(1.0f), alpha(1.0f) {}

void GoalAnimation::start(int teamID) {
    active = true;
    timer = 0.0f;
    scoringTeam = teamID;
    scale = 0.5f;
    alpha = 1.0f;
}

void GoalAnimation::update(float deltaTime) {
    if (!active) return;
    
    timer += deltaTime;
    
    // Animation lasts 2 seconds
    if (timer > 2.0f) {
        active = false;
        return;
    }
    
    // Scale up then down
    if (timer < 0.5f) {
        scale = 0.5f + (timer / 0.5f) * 1.5f; // 0.5 -> 2.0
    } else if (timer > 1.5f) {
        alpha = 1.0f - ((timer - 1.5f) / 0.5f); // Fade out
    }
}

void GoalAnimation::render(SDL_Renderer* renderer, TTF_Font* font) {
    if (!active || !font) return;
    
    const char* text = (scoringTeam == 0) ? "TEAM 1 GOAL!" : "TEAM 2 GOAL!";
    SDL_Color color = (scoringTeam == 0) ? 
        SDL_Color{100, 150, 255, (Uint8)(alpha * 255)} :  // Blue
        SDL_Color{255, 100, 100, (Uint8)(alpha * 255)};   // Red
    
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    if (!surface) return;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    
    // Center and scale text
    int scaledW = (int)(surface->w * scale);
    int scaledH = (int)(surface->h * scale);
    SDL_Rect rect = {
        WINDOW_WIDTH / 2 - scaledW / 2,
        WINDOW_HEIGHT / 3 - scaledH / 2,
        scaledW,
        scaledH
    };
    
    SDL_SetTextureAlphaMod(texture, (Uint8)(alpha * 255));
    SDL_RenderCopy(renderer, texture, nullptr, &rect);
    
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}
