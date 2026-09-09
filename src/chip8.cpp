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