#pragma once

#include <array>
#include <cstdint>
#include <stack>
#include <string>

#define MAX_ROM_SIZE 3584

struct Chip8 {
    std::array<uint8_t, 4096> memory = {}; /** 4 kilobytes of memory */
    std::array<std::array<bool, 64>, 32> display = {}; /** 32 rows, each 64 pixels wide */
    uint16_t PC = 0x200; /** program counter, we use an unsigned 16 bit because want to index our memory. We don't want uint8 because it's too small, and we want an unsigned int because we don't want negatives */
    uint16_t idx_reg = 0; /** index register used to point to locations in memory */
    std::stack<uint16_t> _stack; /** A stack for 16-bit addresses, which is used to call subroutines/functions and return from them */
    uint8_t delay_timer = 0; /** 8 bit delay timer */
    uint8_t sound_timer = 0; /** 8 bit sound timer */ 
    std::array<uint8_t, 16> general_purpose_registers = {}; /** 16 8-bit general purpose variable registers  */

    bool load_rom(const std::string& path);
    void cycle(); /** runs fetch, decode and execute in succession */
    uint16_t fetch(); /** fetches the instruction the PC is currently pointing to in memory  */
    void execute(); /** fetches, decodes and runs one instruction */
};
