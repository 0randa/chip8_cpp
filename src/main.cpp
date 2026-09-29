#include <iostream>

#include "chip8.h"
#include "frontend.h"

/**
 * Instructions to run per frame. At 60 frames a second this gives roughly
 * 660 instructions a second, close to the speed of the original machine.
 * Raise it if games feel slow, lower it if they feel too fast.
 */
constexpr int INSTRUCTIONS_PER_FRAME = 11;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " <rom>\n";
        return 1;
    }

    Chip8 chip8;

    if (!chip8.load_rom(argv[1])) {
        std::cerr << "failed to load rom: " << argv[1] << "\n";
        return 1;
    }

    Frontend frontend;
    frontend.init();

    // One pass of this loop is one frame. SetTargetFPS(60) in init() makes
    // raylib pause at the end of each frame, so it runs 60 times a second.
    while (!frontend.should_close()) {
        // Read keys first, so this frame's instructions see this frame's keys.
        frontend.read_keys(chip8);

        for (int i = 0; i < INSTRUCTIONS_PER_FRAME; i++) {
            chip8.execute();
        }
        chip8.tick_timers();
        frontend.draw(chip8);
    }

    frontend.shutdown();
    return 0;
}
