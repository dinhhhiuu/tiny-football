#pragma once
#include <SDL2/SDL_ttf.h>

// Score system for Tiny Football
struct Score {
    int team1Score;  // Team 1 (Blue) score
    int team2Score;  // Team 2 (Red) score
    
    // Constructor
    Score() : team1Score(0), team2Score(0) {}
    
    // Reset scores
    void reset() {
        team1Score = 0;
        team2Score = 0;
    }
    
    // Add goal for a team (teamID: 0 = Team 1, 1 = Team 2)
    void addGoal(int teamID) {
        if (teamID == 0) {
            team1Score++;
        } else if (teamID == 1) {
            team2Score++;
        }
    }
};
