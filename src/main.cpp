#include <iostream>
#include "chip8.h"
#include "debug.h"

int main(int argc, char* argv[]) {

    Chip8 chip8;

    chip8.load_rom(argv[1]);
    return 0;
}