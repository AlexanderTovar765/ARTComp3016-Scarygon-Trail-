#include "Game.h"

int main(int argc, char** argv) {
    Game game;

    if (!game.init("The Scarygon Trail", 960, 640)) {
        return 1;
    }

    game.run();
    return 0;
}
