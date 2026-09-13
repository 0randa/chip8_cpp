#include "chip8.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <print>
#include <cstdint>
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
    uint8_t first_nibble = (0xF000 & instruction) >> 12;
    switch (first_nibble) {
        case 0x0:
            // clear screen, so we set all the displays to false
            if (instruction == 0x00E0) {
                display = {};
            }
            break;
        case 0x1:
            // jump
            PC = nnn;
            break;
        case 0x3:
            // 3XNN will skip one instruction if the value in VX is equal to NN, and 4XNN will skip if they are not equal.

            if (general_purpose_registers[x] == nn) {
                // std::cout << "Skipping instruction" << std::endl;
                PC += 2;
            }
             
            break;
        case 0x4:
            if (general_purpose_registers[x] != nn) {
                // std::cout << "Skipping instruction" << std::endl;
                PC += 2;
            } 
            // else {
                // std::cout << "Not skipping instruction" << std::endl;
            // }
            break; 
        case 0x6:
            general_purpose_registers[x] = nn;
            break;
        case 0x7:
            general_purpose_registers[x] += nn;
            break;
        case 0x8:
            switch (n) {
                case 0: // 8XY0: Set
                    general_purpose_registers[x] = general_purpose_registers[y];
                    break;
                case 1:
                    general_purpose_registers[x] |= general_purpose_registers[y];
                    break;
                case 2: // BINARY AND
                    general_purpose_registers[x] &= general_purpose_registers[y];
                    break;
                case 3: // 8XY3: logical XOR. No flag.
                    general_purpose_registers[x] ^= general_purpose_registers[y];
                    break;
                case 4: { // 8XY4: VX += VY, VF = 1 on overflow past 255
                    // Widen to 16 bits so the sum isn't truncated before we can
                    // test it. uint8_t arithmetic would wrap and hide the carry.
                    uint16_t sum = static_cast<uint16_t>(general_purpose_registers[x])
                                 + general_purpose_registers[y];
                    uint8_t carry = (sum > 0xFF) ? 1 : 0; // sum is greater than 255
                    general_purpose_registers[x] = static_cast<uint8_t>(sum);

                    // VF is written last: if x == 0xF, the flag must win.
                    general_purpose_registers[0xF] = carry;
                    break;
                }
                case 5: { // 8XY5: VX = VX - VY, VF = 1 when there is NO borrow
                    uint8_t no_borrow =
                        (general_purpose_registers[x] >= general_purpose_registers[y]) ? 1 : 0;
                    general_purpose_registers[x] = static_cast<uint8_t>(
                        general_purpose_registers[x] - general_purpose_registers[y]);
                    general_purpose_registers[0xF] = no_borrow;
                    break;
                }
                case 7: { // 8XY7: VX = VY - VX, VF = 1 when there is NO borrow
                    uint8_t no_borrow =
                        (general_purpose_registers[y] >= general_purpose_registers[x]) ? 1 : 0;
                    general_purpose_registers[x] = static_cast<uint8_t>(
                        general_purpose_registers[y] - general_purpose_registers[x]);
                    general_purpose_registers[0xF] = no_borrow;
                    break;
                }
                case 6: { // 8XY6
                    // shift the value of VX one bit to the right
                    uint8_t fall_off = general_purpose_registers[x] & 1;
                    general_purpose_registers[x] >>= 1;
                    
                    if (fall_off == 1) {
                        general_purpose_registers[0xF] = 1;
                    } else {
                        general_purpose_registers[0xF] = 0;
                    }

                    break;
                }
                case 14: { // 8XYE
                    uint8_t fall_off = general_purpose_registers[x] & 0x80;
                    general_purpose_registers[x] <<= 1;
                    
                    if (fall_off != 0) {
                        general_purpose_registers[0xF] = 1;
                    } else {
                        general_purpose_registers[0xF] = 0;
                    }
                    break;
                }
            }
            break;
        case 0xA:
            idx_reg = nnn;
            break;
        case 0xD: {
            // draw something

            // DXYN

            /**
             * X = The index to get the horizontal position in VX
             * Y = The index to get the vertical position stored in VY
             * N = num rows
             */

            //  A sprite is a stamp that is always 8 pixels wide and N rows tall

            uint8_t col_num = general_purpose_registers[x] % 64;
            uint8_t row_num = general_purpose_registers[y] % 32;
            
            general_purpose_registers[0xF] = 0; // VF = 0;

            for (uint8_t i = 0; i < n; i++) {
                uint8_t sprite_byte = memory[idx_reg + i]; // get the nth byte of sprite data
                
                // for the 8 pixels (bits) in the sprite row:
                
                // increment Y;
                if (row_num + i > 31) {
                   break;
                }

                // 0x80 = 1 0 0 0 0 0 0 0
                int j = 0;
                for (uint8_t mask = 0x80; mask != 0; mask = mask >> 1) {
                    
                    bool curr_pixel = (sprite_byte & mask) != 0;

                    if (col_num + j > 63) {
                        break;
                    }
                    // current pixel is on, and the coords at display[x][y] are also on
                    // turn off the pixel and set VF to 1.
                    if (curr_pixel && display[row_num + i][col_num + j]) {
                        // turn the pixel off
                        display[row_num + i][col_num + j] = false; 
                        general_purpose_registers[0xF] = 1;
                    } else if (curr_pixel && !display[row_num + i][col_num + j]) {
                        // draw the pixel at the coords
                        display[row_num + i][col_num + j] = true; 
                    } 
                    // at the right edge of the screen

                    j += 1;
                }

            
               
            }

            break;
        }
        default:
            // print the unrecognised opcode in hex

            std::cout << "Unrecognised opcode: " << "0x" 
              << std::hex          // Switch stream to hexadecimal mode
              << std::setfill('0') // Use '0' for padding
              << std::setw(4)      // Expect exactly 4 characters
              << instruction << std::endl;  
            break;
    }
}


void Chip8::render() {

    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 64; j++) { 

            if (display[i][j]) {
                std::cout << "#";
            } else {
                std::cout << " ";
            }

        }
        std::cout << std::endl;
    }
}