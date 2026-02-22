#include "game.h"

int main(int, char**) {
    Game game;

    if (!game.init()) return -1;

    game.run();
    game.clean();

    return 0;
}
