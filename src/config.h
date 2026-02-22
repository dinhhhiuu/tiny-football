#pragma once

// ===== Window =====
#define WINDOW_TITLE   "Tiny Football"
#define WINDOW_WIDTH   800
#define WINDOW_HEIGHT  600

// ===== FPS =====
#define TARGET_FPS     60
#define FRAME_DELAY    (1000 / TARGET_FPS)

// ===== Match =====
#define MATCH_TIME_SECONDS  60.0f  

// ===== Colors =====
#define COLOR_MENU_BG_R   30
#define COLOR_MENU_BG_G   30
#define COLOR_MENU_BG_B   30

#define COLOR_FIELD_R     0
#define COLOR_FIELD_G     130
#define COLOR_FIELD_B     0

// ===== Field =====
#define FIELD_MARGIN     50
#define FIELD_LEFT       FIELD_MARGIN
#define FIELD_RIGHT      (WINDOW_WIDTH - FIELD_MARGIN)
#define FIELD_TOP        FIELD_MARGIN
#define FIELD_BOTTOM     (WINDOW_HEIGHT - FIELD_MARGIN)

// ===== Player =====
#define PLAYER_SIZE       40
#define PLAYER_SPEED      250.0f  // pixels per second
#define PLAYERS_PER_TEAM  3        // 3v3 game
#define TOTAL_PLAYERS     (PLAYERS_PER_TEAM * 2)

// Player struct for Task B
struct Player {
    float x;          // position X
    float y;          // position Y
    int w;            // width
    int h;            // height
    float speed;      // movement speed
    int playerID;     // 0-2 = Team 1, 3-5 = Team 2
    int teamID;       // 0 = Team 1, 1 = Team 2
    bool isActive;    // Currently controlled by player
    
    // Colors
    unsigned char r;
    unsigned char g;
    unsigned char b;
};