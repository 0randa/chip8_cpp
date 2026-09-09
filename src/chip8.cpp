#include "chip8.h"
#include <filesystem>
#include <fstream>
#include <iostream>
// #include "debug.h"

bool Chip8::load_rom(const std::string& path) {
    // open the file

    if (!std::filesystem::exists(path)) return false;

    uint16_t file_size = std::filesystem::file_size(path);
    
    // check the file size is not greater than file size - offset (4096 - 512)
    if (file_size > MAX_ROM_SIZE) {
        return false;
    }

    std::ifstream in;
    in.open(path, std::ios::binary);
    int offset = 0x200;
    int ch;

    while ((ch = in.get()) != EOF) {
        memory[offset] = ch;
        offset += 1;
    }

    in.close();
    return true;
    // then chuck the bytes into memory starting from 512
}

uint16_t Chip8::fetch() {

    // typecast the first to 16 bits first
    uint16_t res = (static_cast<uint16_t> (memory[PC]) << 8) | (memory[PC+1]);

    PC += 2;

    return res;
}

void Chip8::execute() {
    // fetch
    uint16_t instruction = fetch();

    // the decoding step
    uint8_t x = (0x0F00 & instruction) >> 8; /** 2nd nibble for looking at one of the 16 registers */
    uint8_t y = (0x00F0 & instruction) >> 4; /** 3rd nibble, similar role to x */
    uint8_t n = 0x000F & instruction; /** 4th nibble */
    uint8_t nn = 0x00FF & instruction; /** 2nd byte, i.e. 3rd and 4th nibbles */
    uint16_t nnn = 0x0FFF & instruction; /** 2nd, 3rd and 4th nibbles */

    // execute
}