#include <iostream>
#include "chip8.h"

int main() {

    Chip8 chip8;

    std::cout << sizeof(chip8) << std::endl;
    std::cout << chip8.PC << std::endl;
    // std::cout << chip8.memory[0] << std::endl;
    std::cout << static_cast<int>(chip8.memory[0]) << std::endl;
    std::cout << static_cast<int>(chip8.memory[4095]) << std::endl;
    return 0;
}