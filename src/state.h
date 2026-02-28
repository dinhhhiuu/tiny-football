#pragma once

enum class GameState {
    START,
    MENU,
    PLAY,
    RESULTS,
    EXIT
};

enum class GameMode {
    PVP,
    PVE,
};

enum class Corner {
    NONE,
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT
};