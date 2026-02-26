// Implementations for Score
#include "score.h"

Score::Score() : team1Score(0), team2Score(0) {}

void Score::reset() {
    team1Score = 0;
    team2Score = 0;
}

void Score::addGoal(int teamID) {
    if (teamID == 0) {
        team1Score++;
    } else if (teamID == 1) {
        team2Score++;
    }
}
