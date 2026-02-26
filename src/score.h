#pragma once

// Score system for Tiny Football
struct Score {
    int team1Score;  // Team 1 (Blue) score
    int team2Score;  // Team 2 (Red) score

    // Constructor
    Score();

    // Reset scores
    void reset();

    // Add goal for a team (teamID: 0 = Team 1, 1 = Team 2)
    void addGoal(int teamID);
};
