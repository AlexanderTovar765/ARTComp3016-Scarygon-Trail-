#include <SDL3/SDL_main.h>
#include "Game.h"

//this part of the code is the programs entry point
int main(int /*argc*/, char** /*argv*/) {
    Game game;

    if (!game.init("The Scarygon Trail", 960, 640)) {
        return 1;
    }

    game.run();
    return 0;
}
