#include "game.h"
#include "AI.h"

#include <cmath>
#include <iostream>

// Define the global corner state for AI usage
Corner currentCorner = Corner::NONE;

void updateExtendedAI(float deltaTime, Player players[], Ball& ball) {
    // Team 1 goals: left side
    float ownGoalX = FIELD_LEFT - 20.0f;
    float ownGoalY = WINDOW_HEIGHT / 2.0f;
    float fieldMidX = (FIELD_LEFT + FIELD_RIGHT) * 0.5f;

    // global AI speed multiplier (reduce to make AI slower)
    const float aiSpeedMult = 0.5f;

    // ----- Player 0: normally position between ball and own goal; if ball in AI half, dash to ball -----
    {
        int i = 0;
        float px = players[i].x + players[i].w / 2.0f;
        float py = players[i].y + players[i].h / 2.0f;

        // corner
        if (currentCorner != Corner::NONE && (std::sqrt((ball.x - px)*(ball.x - px) + (ball.y - py)*(ball.y - py)) < 50.0f)) {
            // Move towards center of field to escape corner
            float targetX = WINDOW_WIDTH / 2.0f;
            float targetY = WINDOW_HEIGHT / 2.0f;
            float dx = targetX - px;
            float dy = targetY - py;
            float dist = std::sqrt(dx*dx + dy*dy);
            if (dist > 1.0f) {
                players[i].x += (dx / dist) * players[i].speed * aiSpeedMult * deltaTime;
                players[i].y += (dy / dist) * players[i].speed * aiSpeedMult * deltaTime;
            }
        } else {
            if (ball.x < fieldMidX) {
                // Dash to ball aggressively but clamped to left half
                float dx = ball.x - px;
                float dy = ball.y - py;
                float dist = std::sqrt(dx*dx + dy*dy);
                float kickRange = 28.0f + ball.radius;
                if (dist > 1.0f) {
                    float dashSpeed = 1.4f * aiSpeedMult; // faster dash
                    players[i].x += (dx / dist) * players[i].speed * dashSpeed * deltaTime;
                    players[i].y += (dy / dist) * players[i].speed * dashSpeed * deltaTime;
                }
                // If within kicking range and ball is in AI half, kick it away from the player
                if (dist <= kickRange) {
                    // Only kick if ball is in front of player (to the right for Team 1)
                    // require the ball's left edge to be strictly in front of player's center
                    if (ball.x - ball.radius > px) {
                        // direction from player to ball
                        float nx = ball.x - px;
                        float ny = ball.y - py;
                        float nlen = std::sqrt(nx*nx + ny*ny);
                        if (nlen < 0.001f) nlen = 1.0f;
                        nx /= nlen; ny /= nlen;

                        float kickForce = 420.0f; // moderate kick force
                        std::cout << "[AI KICK] p0 center=" << px << " ball.center=" << ball.x << " ball.left=" << (ball.x - ball.radius) << " dist=" << dist << "\n";
                        ball.kick(nx, ny, kickForce);
                    }
                }
                // clamp to left half
                float margin = 20.0f;
                if (players[i].x + players[i].w/2.0f > fieldMidX - margin) {
                    players[i].x = fieldMidX - margin - players[i].w/2.0f;
                }
            } else {
                // default behavior: position between ball and own goal
                float gx = ownGoalX - ball.x;
                float gy = ownGoalY - ball.y;
                float glen = std::sqrt(gx*gx + gy*gy);
                if (glen < 0.0001f) glen = 1.0f;
                gx /= glen; gy /= glen;

                float offset = 80.0f; // distance from ball towards own goal
                float targetX = ball.x + gx * offset;
                float targetY = ball.y + gy * offset;

                if (targetX + players[i].w/2.0f > fieldMidX - 40.0f) {
                    targetX = fieldMidX - 40.0f - players[i].w/2.0f;
                }

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
    }

    // ----- Players 1 and 2: dynamic chaser/supporter behavior -----
    {
        int a = 1, b = 2;
        float ax = players[a].x + players[a].w/2.0f;
        float ay = players[a].y + players[a].h/2.0f;
        float bx = players[b].x + players[b].w/2.0f;
        float by = players[b].y + players[b].h/2.0f;

        float distA = std::sqrt((ball.x - ax)*(ball.x - ax) + (ball.y - ay)*(ball.y - ay));
        float distB = std::sqrt((ball.x - bx)*(ball.x - bx) + (ball.y - by)*(ball.y - by));

        int chaser = (distA < distB) ? a : b;
        int supporter = (chaser == a) ? b : a;

        if (currentCorner != Corner::NONE && (distA < 50.0f || distB < 50.0f)) {
            // If ball is in corner, both players should try to move towards center of field to escape corner
            int i1 = chaser, i2 = supporter;
            float targetX = WINDOW_WIDTH / 2.0f;
            float targetY = WINDOW_HEIGHT / 2.0f;

            for (int i : {i1, i2}) {
                float px = players[i].x + players[i].w/2.0f;
                float py = players[i].y + players[i].h/2.0f;
                float dx = targetX - px;
                float dy = targetY - py;
                float dist = std::sqrt(dx*dx + dy*dy);
                if (dist > 1.0f) {
                    players[i].x += (dx / dist) * players[i].speed * aiSpeedMult * deltaTime;
                    players[i].y += (dy / dist) * players[i].speed * aiSpeedMult * deltaTime;
                }
            }
        }

        // --- Chaser: pursue ball and kick when in range ---
        {
            int i = chaser;
            float px = players[i].x + players[i].w/2.0f;
            float py = players[i].y + players[i].h/2.0f;
            float dx = ball.x - px;
            float dy = ball.y - py;
            float dist = std::sqrt(dx*dx + dy*dy);
            float kickRange = 28.0f + ball.radius;

            if (dist <= kickRange) {
                float pxCenter = players[i].x + players[i].w/2.0f;
                // require the ball's left edge to be in front of player's center
                if (ball.x - ball.radius > pxCenter) {
                    float oppGoalX = FIELD_RIGHT + 30.0f;
                    float oppGoalY = WINDOW_HEIGHT / 2.0f;
                    float gx = oppGoalX - ball.x;
                    float gy = oppGoalY - ball.y;
                    float glen = std::sqrt(gx*gx + gy*gy);
                    if (glen < 0.001f) glen = 1.0f;
                    gx /= glen; gy /= glen;
                    float kickForce = 380.0f;
                    std::cout << "[AI KICK] p" << i << " center=" << pxCenter << " ball.center=" << ball.x << "\n";
                    ball.kick(gx, gy, kickForce);
                } else {
                    if (dist > 0.5f) {
                        float speedFactor = aiSpeedMult;
                        players[i].x += (dx / dist) * players[i].speed * speedFactor * deltaTime;
                        players[i].y += (dy / dist) * players[i].speed * speedFactor * deltaTime;
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

        // --- Supporter: mirror chaser to create passing lane / provide support ---
        {
            int i = supporter;
            int lead = chaser;
            float leadX = players[lead].x + players[lead].w/2.0f;
            float leadY = players[lead].y + players[lead].h/2.0f;
            float centerY = WINDOW_HEIGHT / 2.0f;
            float mirroredY = 2.0f * centerY - leadY;
            float smallOffsetX = 20.0f;
            float targetX = leadX + smallOffsetX;
            float targetY = mirroredY;

            float px = players[i].x + players[i].w/2.0f;
            float py = players[i].y + players[i].h/2.0f;
            float dx = targetX - px;
            float dy = targetY - py;
            float dist = std::sqrt(dx*dx + dy*dy);

            float distToBall = std::sqrt((ball.x - px)*(ball.x - px) + (ball.y - py)*(ball.y - py));
            float kickRange = 28.0f + ball.radius;

            if (distToBall <= kickRange) {
                float pxCenter = players[i].x + players[i].w/2.0f;
                if (ball.x - ball.radius > pxCenter) {
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
                    if (dist > 0.5f) {
                        float speedFactor = aiSpeedMult * 0.9f;
                        players[i].x += (dx / dist) * players[i].speed * speedFactor * deltaTime;
                        players[i].y += (dy / dist) * players[i].speed * speedFactor * deltaTime;
                    }
                }
            } else {
                if (dist > 1.0f) {
                    float speedFactor = aiSpeedMult * 0.9f;
                    players[i].x += (dx / dist) * players[i].speed * speedFactor * deltaTime;
                    players[i].y += (dy / dist) * players[i].speed * speedFactor * deltaTime;
                }
            }
        }
    }
}

// Team 2 AI helper when human controls one Team 2 player in PVE
void updateExtendedTeam2AI(float deltaTime, Player players[], Ball& ball, int activePlayerTeam2) {
    // active player is controlled by human: activePlayerTeam2
    int active = activePlayerTeam2; // should be 3..5
    // define roles based on which player is active
    int support = 3, defend = 4; // defaults
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
        float centerY = WINDOW_HEIGHT / 2.0f;
        float mirroredY = 2.0f * centerY - leadY;
        float smallOffsetX = -20.0f; // move slightly toward opponent (left)
        float targetX = leadX + smallOffsetX;
        float targetY = mirroredY;

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
        float ownGoalX = FIELD_RIGHT + 20.0f;
        float ownGoalY = WINDOW_HEIGHT / 2.0f;
        float gx = ownGoalX - ball.x;
        float gy = ownGoalY - ball.y;
        float glen = std::sqrt(gx*gx + gy*gy);
        if (glen < 0.0001f) glen = 1.0f;
        gx /= glen; gy /= glen;
        float offset = 80.0f;
        float targetX = ball.x + gx * offset;
        float targetY = ball.y + gy * offset;

        // Prevent defender from wandering past midfield (keep on right half)
        float fieldMidX = (FIELD_LEFT + FIELD_RIGHT) * 0.5f;
        float halfW = players[i].w / 2.0f;
        float minCenterX = fieldMidX + 40.0f; // defender shouldn't move left of this center X
        if (targetX - halfW < minCenterX) {
            targetX = minCenterX + halfW;
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
