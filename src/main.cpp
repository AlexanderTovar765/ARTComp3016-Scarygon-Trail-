<<<<<<< HEAD
#include <SDL3/SDL_main.h>
#include "Game.h"

int main(int /*argc*/, char** /*argv*/) {
=======
#include "Game.h"

int main(int argc, char** argv) {
>>>>>>> 8335505f5d66bfc5275628be58ae86571016e2e1
    Game game;

    if (!game.init("The Scarygon Trail", 960, 640)) {
        return 1;
    }

    game.run();
    return 0;
}
