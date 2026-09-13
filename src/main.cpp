#include <cstdlib>
#include <iostream>

#include "chip8.h"

/** How many instructions to run when no count is given on the command line. */
constexpr int DEFAULT_CYCLES = 5000;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " <rom> [cycles]\n";
        return 1;
    }

    int cycles = (argc >= 3) ? std::atoi(argv[2]) : DEFAULT_CYCLES;

    Chip8 chip8;

    if (!chip8.load_rom(argv[1])) {
        std::cerr << "failed to load rom: " << argv[1] << "\n";
        return 1;
    }

    for (int i = 0; i < cycles; i++) {
        chip8.execute();
    }

    return 0;
}
