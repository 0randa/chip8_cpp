#include <iostream>
#include "chip8.h"
#include "debug.h"

int main(int argc, char* argv[]) {

    Chip8 chip8;

    chip8.load_rom(argv[1]);

    for (int i =0; i < 30; i++) {
        chip8.execute();
    }
    chip8.render();

    return 0;
}