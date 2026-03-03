#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>

// Single particle
struct Particle {
    float x, y;
    float vx, vy;
    float lifetime;
    float maxLifetime;
    Uint8 r, g, b;
    float size;
};

// Particle system for effects
class ParticleSystem {
public:
    ParticleSystem();
    
    // Create particles at a position
    void createKickEffect(float x, float y, float ballVx, float ballVy);
    void createGoalExplosion(float x, float y);
    
    // Update and render
    void update(float deltaTime);
    void render(SDL_Renderer* renderer);
    
    void clear();
    
private:
    std::vector<Particle> particles;
    const int MAX_PARTICLES = 500;
};

// Goal celebration animation
class GoalAnimation {
public:
    GoalAnimation();
    
    void start(int teamID);
    void update(float deltaTime);
    void render(SDL_Renderer* renderer, TTF_Font* font);
    bool isActive() const { return active; }
    
private:
    bool active;
    float timer;
    int scoringTeam;
    float scale;
    float alpha;
};
