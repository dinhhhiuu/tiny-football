#include "game.h"
#include "AI.h"

#include <cmath>
#include <iostream>
#include <algorithm>

// Define the global corner state for AI usage
Corner currentCorner = Corner::NONE;
// random tweak for AI decisions
float randomOffset = (rand() % 100 - 50) * 0.1f; // -5 to +5 pixels

void applyFieldBoundary(Player& p, float deltaTime, Ball& ball) {
    float safeMargin = 10.0f;   // Minimum distance from field edge to avoid getting stuck
    float pushStrength = 150.0f;

    float leftLimit = FIELD_LEFT + safeMargin;
    float rightLimit = FIELD_RIGHT - safeMargin;

    if (minimumAvoidBoundary[p.playerID] > 0) {
        p.x += pushStrength * deltaTime * (p.x < (FIELD_LEFT + FIELD_RIGHT) / 2.0f ? 1.0f : -1.0f);
        minimumAvoidBoundary[p.playerID]--;
        std::cout << "[AI] Player " << p.playerID << " avoiding boundary, push applied. Remaining: " << minimumAvoidBoundary[p.playerID] << "\n";
    } 
    else if (p.x < leftLimit && ball.x - ball.radius < leftLimit) {  
        minimumAvoidBoundary[p.playerID] = 25;
    }
    else if (p.x + p.w > rightLimit && ball.x + ball.radius > rightLimit) {
        minimumAvoidBoundary[p.playerID] = 25;
    }
}

void updateExtendedAI(float deltaTime, Player players[], Ball& ball) {
    // Assign roles based on distance to the ball: 
    // nearest -> chaser, second -> supporter, farthest -> defender

    const float aiSpeedMult = 0.6f;

    // Compute center positions and distances for players 0..2
    struct P { int idx; float cx; float cy; float dist; } arr[3];
    
    for (int i = 0; i < 3; ++i) {
        arr[i].idx = i;
        arr[i].cx = players[i].x;
        arr[i].cy = players[i].y;
        float dx = ball.x - arr[i].cx;
        float dy = ball.y - arr[i].cy;
        arr[i].dist = std::sqrt(dx*dx + dy*dy);
    }

    int chaser = 2, supporter = 1, defender = 0; // default roles

    int idxs[3] = {0, 1, 2};
    std::sort(idxs, idxs + 3, [&](int a, int b) {
        return arr[a].dist < arr[b].dist;
    });

    if (idxs[0] == 0) {
        chaser = 0; supporter = 1; defender = 2;
    } else if (idxs[0] == 1) {
        chaser = 1; supporter = 0; defender = 2;
    } else {
        chaser = 2; supporter = 0; defender = 1;
    }

    // --- Chaser behavior: pursue ball and kick toward opponent goal when close ---
    {
        int i = chaser;
        float px = players[i].x + players[i].w/2.0f;
        float py = players[i].y + players[i].h/2.0f;
        float dx = ball.x - px;
        float dy = ball.y - py;
        float dist = std::sqrt(dx*dx + dy*dy);
        float kickRange = 28.0f + ball.radius;

        // Close boundary
        applyFieldBoundary(players[i], deltaTime, ball);
        
        if (dist <= kickRange) {
            float pxCenter = players[i].x + players[i].w/2.0f;
            if (ball.x - ball.radius > pxCenter && ball.y < players[i].y) {
                float oppGoalX = FIELD_RIGHT + 30.0f;
                float oppGoalY = WINDOW_HEIGHT / 2.0f;
                float gx = oppGoalX - ball.x;
                float gy = oppGoalY - ball.y;
                float glen = std::sqrt(gx*gx + gy*gy);
                if (glen < 0.001f) glen = 1.0f;
                gx /= glen; gy /= glen;
                if (rand() % 100 < 50) {
                    // Add some random offset to make AI less perfect
                    gx += (randomOffset * 0.5f);
                    gy += (randomOffset * 0.5f);
                    float newLen = std::sqrt(gx*gx + gy*gy);
                    if (newLen > 0.0001f) {
                        gx /= newLen; gy /= newLen;
                    }
                }
                float kickForce = 380.0f;
                std::cout << "[AI KICK] p" << i << " center=" << pxCenter << " ball.center=" << ball.x << "\n";
                ball.kick(gx, gy, kickForce);
            } else {
                if (currentCorner == Corner::NONE) {
                    float speedFactor = aiSpeedMult;
                    players[i].x += (dx / dist) * players[i].speed * speedFactor * deltaTime;
                    players[i].y += (dy / dist) * players[i].speed * speedFactor * deltaTime;
                } else {
                    // If in corner, move towards center to escape
                    float targetX = WINDOW_WIDTH / 2.0f;
                    float targetY = WINDOW_HEIGHT / 2.0f;
                    float dx = targetX - px;
                    float dy = targetY - py;
                    float dist = std::sqrt(dx*dx + dy*dy);
                    if (dist > 1.0f) {
                        players[i].x += (dx / dist) * players[i].speed * aiSpeedMult * deltaTime * 10.0f; // move faster when escaping corner
                        players[i].y += (dy / dist) * players[i].speed * aiSpeedMult * deltaTime * 10.0f;
                    }
                }
            }
        } else {
            if (dist > 1.0f) {
                float speedFactor = aiSpeedMult;
                players[i].x += (dx / dist) * players[i].speed * speedFactor * deltaTime;
                players[i].y += (dy / dist) * players[i].speed * speedFactor * deltaTime;
            }
        }
    }

    // --- Supporter behavior: mirror chaser to create passing lane / provide support ---
    {
        // info
        int i = supporter;
        int lead = chaser;

        float leadX = players[lead].x;
        float leadY = players[lead].y;

        float mirroredY = WINDOW_HEIGHT - leadY;
        float smallOffsetX = 20.0f;

        float targetX = leadX + smallOffsetX + randomOffset;
        float targetY = mirroredY + randomOffset;

        float px = players[i].x;
        float py = players[i].y;
        float dx = targetX - px;
        float dy = (targetY - py) - 10.0f * randomOffset; // move slower vertically to create better passing angle
        float dist = std::sqrt(dx*dx + dy*dy);

        float distToBall = std::sqrt((ball.x - px)*(ball.x - px) + (ball.y - py)*(ball.y - py));
        float kickRange = 28.0f + ball.radius;

        applyFieldBoundary(players[i], deltaTime, ball);

        // If supporter is closer to ball than chaser, try to kick. Otherwise, move to mirrored position
        if (distToBall <= kickRange) {
            float pxCenter = players[i].x + players[i].w/2.0f;
            if (ball.x - ball.radius > pxCenter && ball.y < players[i].y) {
                float oppGoalX = FIELD_RIGHT + 30.0f;
                float oppGoalY = WINDOW_HEIGHT / 2.0f;
                float gx = oppGoalX - ball.x;
                float gy = oppGoalY - ball.y;
                float glen = std::sqrt(gx*gx + gy*gy);
                if (glen < 0.001f) glen = 1.0f;
                gx /= glen; gy /= glen;
                float kickForce = 350.0f;
                std::cout << "[AI KICK] p" << i << " center=" << pxCenter << " ball.center=" << ball.x << "\n";
                ball.kick(gx, gy, kickForce);
            } else {
                if (distToBall > 0.5f) {
                    float speedFactor = aiSpeedMult * 0.9f;
                    players[i].x += (dx / dist) * players[i].speed * speedFactor * deltaTime;
                    players[i].y += (dy / dist) * players[i].speed * speedFactor * deltaTime;
                }
            }
        } else { // Move to mirrored support position
            if (dist > 1.0f) {
                float speedFactor = aiSpeedMult * 0.9f;
                players[i].x += (dx / dist) * players[i].speed * speedFactor * deltaTime;
                players[i].y += (dy / dist) * players[i].speed * speedFactor * deltaTime;
            }
        }
    }

    // --- Defender behavior: position between ball and own goal (left side) ---
    {
        int i = defender;
        float px = players[i].x + players[i].w/2.0f;
        float py = players[i].y + players[i].h/2.0f;
        float ownGoalX = FIELD_LEFT - 20.0f;
        float ownGoalY = WINDOW_HEIGHT / 2.0f;
        float gx = ownGoalX - ball.x;
        float gy = ownGoalY - ball.y;
        float glen = std::sqrt(gx*gx + gy*gy);
        if (glen < 0.0001f) glen = 1.0f;
        gx /= glen; gy /= glen;
        float offset = 80.0f;
        float targetX = ball.x + gx * offset;
        float targetY = ball.y + gy * offset;

        applyFieldBoundary(players[i], deltaTime, ball);

        float dx = targetX - px;
        float dy = targetY - py;
        float dist = std::sqrt(dx*dx + dy*dy);
        if (dist > 1.0f) {
            float speedFactor = 0.9f * aiSpeedMult;
            players[i].x += (dx / dist) * players[i].speed * speedFactor * deltaTime;
            players[i].y += (dy / dist) * players[i].speed * speedFactor * deltaTime;
        }
    }
}

// Team 2 AI helper when human controls one Team 2 player in PVE
void updateExtendedTeam2AI(float deltaTime, Player players[], Ball& ball, int activePlayerTeam2) {

    int active = activePlayerTeam2; 
    int support = 3, defend = 4; 
    if (active == 3) { support = 4; defend = 5; }
    else if (active == 4) { support = 5; defend = 3; }
    else if (active == 5) { support = 3; defend = 4; }

    // Ensure non-active players act as AI
    for (int i = 3; i <=5; ++i) players[i].isAI = (i != active);

    // Supporter: mirror active to create passing lane
    {
        int i = support;

        float leadX = players[active].x + players[active].w/2.0f;
        float leadY = players[active].y + players[active].h/2.0f;

        float defenderX = players[defend].x + players[defend].w/2.0f;
        float defenderY = players[defend].y + players[defend].h/2.0f;

        float targetTempX, targetTempY;
        
        // home feild
        if (WINDOW_WIDTH / 2.0f < ball.x) {
            targetTempX = ((leadX < defenderX) ? defenderX : leadX);
            targetTempY =  ((leadY < defenderY) ? defenderY : leadY);
        } else { // away feild
            targetTempX = ((leadX < defenderX) ? leadX : defenderX);
            targetTempY = ((leadY < defenderY) ? leadY : defenderY);
        }

        float centerY = WINDOW_HEIGHT / 2.0f;
        float mirroredY = 2.0f * centerY - targetTempY;
        float smallOffsetX = -20.0f; 

        float targetX = targetTempX + smallOffsetX;
        float targetY = mirroredY - 50.0f * ((targetTempY < centerY) ? 1.0f : -1.0f);

        float px = players[i].x + players[i].w/2.0f;
        float py = players[i].y + players[i].h/2.0f;

        float dx = targetX - px;
        float dy = targetY - py;

        float dist = std::sqrt(dx*dx + dy*dy);

        if (dist > 1.0f) {
            float speedFactor = 0.9f * 0.5f;
            players[i].x += (dx / dist) * players[i].speed * speedFactor * deltaTime;
            players[i].y += (dy / dist) * players[i].speed * speedFactor * deltaTime;
        }
    }

    // Defender: position between ball and own goal (right side)
    {
        int i = defend;

        float px = players[i].x + players[i].w/2.0f;
        float py = players[i].y + players[i].h/2.0f;

        // Calculate a point that is offset from the ball in the direction of the own goal
        float fieldWidth = FIELD_RIGHT - FIELD_LEFT;
        float dangerLine = FIELD_RIGHT - fieldWidth * 0.25f;

        float targetX, targetY;

        // Nếu bóng vào 1/4 sân → lao lên
        if (ball.x >= dangerLine && ball.y > FIELD_TOP + fieldWidth * 0.15f && ball.y < FIELD_BOTTOM - fieldWidth * 0.15f) {

            targetX = ball.x + ((ball.x < px) ? -abs(randomOffset) : abs(randomOffset)); // offset to left or right of ball
            targetY = ball.y;

        } 
        // Bình thường → đứng chắn
        else {

            float ownGoalX = FIELD_RIGHT + 20.0f;
            float ownGoalY = WINDOW_HEIGHT / 2.0f;

            float gx = ownGoalX - ball.x;
            float gy = ownGoalY - ball.y;

            float glen = std::sqrt(gx*gx + gy*gy);
            if (glen < 0.0001f) glen = 1.0f;

            gx /= glen;
            gy /= glen;

            float offset = 80.0f;

            targetX = ball.x + gx * offset;
            targetY = ball.y + gy * offset;
        }

        float dx = targetX - px;
        float dy = targetY - py;
        float dist = std::sqrt(dx*dx + dy*dy);
        if (dist > 1.0f) {
            float speedFactor = 0.9f * 0.5f;
            players[i].x += (dx / dist) * players[i].speed * speedFactor * deltaTime;
            players[i].y += (dy / dist) * players[i].speed * speedFactor * deltaTime; 
        }
    }
}