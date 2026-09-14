#include "chip8.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <print>
#include <cstdint>
#include <random>
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
        case 0x0: {

            // clear screen, so we set all the displays to false
            if (instruction == 0x00E0) {
                display = {};
            } else if (instruction == 0x00EE) {
                if (!_stack.empty()) {
                    PC = _stack.top();
                    _stack.pop();
                }
            }
            break;
        }
        case 0x1: {
            // jump
            PC = nnn;
            break;
        }
        case 0x2: {

            _stack.push(PC);
            PC = nnn;
            break;
        }
        case 0x3: {

            // 3XNN will skip one instruction if the value in VX is equal to NN, and 4XNN will skip if they are not equal.
            
            if (general_purpose_registers[x] == nn) {
                // std::cout << "Skipping instruction" << std::endl;
                PC += 2;
            }
            
            break;
        }
        case 0x4: {

            if (general_purpose_registers[x] != nn) {
                // std::cout << "Skipping instruction" << std::endl;
                PC += 2;
            } 
            // else {
                // std::cout << "Not skipping instruction" << std::endl;
                // }
                break; 
            }
        case 0x5: {

            // 0x5XY0 skips if the values in VX and VY are equal
            if (general_purpose_registers[x] == general_purpose_registers[y]) {
                PC += 2;
            }
            break;
        }
        case 0x6: {

            general_purpose_registers[x] = nn;
            break;
        }
        case 0x7: {

            general_purpose_registers[x] += nn;
            break;
        }
        case 0x8: {

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
        }
        case 0x9: {

            // 0x9XY0 skips if VX and VY are not equal
            if (general_purpose_registers[x] != general_purpose_registers[y]) {
                PC += 2;
            } 
            break;
        }
        case 0xA: {

            idx_reg = nnn;
            break;
        }
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
        case 0xF: {
            switch (nn) {
                case 0x07: // FX07: VX = current delay timer value
                    general_purpose_registers[x] = delay_timer;
                    break;
                case 0x15: // FX15: delay timer = VX
                    delay_timer = general_purpose_registers[x];
                    break;
                case 0x18: // FX18: sound timer = VX (machine beeps while > 0)
                    sound_timer = general_purpose_registers[x];
                    break;
                case 0x1E: // FX1E: I += VX
                    idx_reg = static_cast<uint16_t>(idx_reg + general_purpose_registers[x]);
                    break;
                case 0x33: {
                    /** It takes the number in VX (which is one byte, so it can
                     * be any number from 0 to 255) and converts it to three
                     * decimal digits, storing these digits in memory at the
                     * address in the index register I.  */

                    int first = general_purpose_registers[x] / 100;       // 582 / 100 = 5
                    
                    memory[idx_reg] = first;
                    // 2. Get the second digit (Tens place)
                    int second = (general_purpose_registers[x] / 10) % 10; // 582 / 10 = 58 -> 58 % 10 = 8
                    
                    memory[idx_reg + 1] = second;

                    // 3. Get the third digit (Ones place)
                    int third = general_purpose_registers[x] % 10; 
                    
                    memory[idx_reg + 2] = third;

                    break;
                }
                case 0x55: {
                    /**
                     * the value of each variable register from V0 to VX
                     * INCLUSIVE (if X is 0, then only V0) will be stored
                     * in successive memory addresses, starting with the
                     * one that’s stored in I
                     */

                    for (uint8_t i = 0; i <= x; i++) {
                        memory[idx_reg + i] = general_purpose_registers[i];
                    }
                    break;
                }

                case 0x65: {
                    /*
                        it takes the value stored at the memory addresses and loads
                        them into the variable registers instead.
                    */
                    for (uint8_t i = 0; i <= x; i++) {
                        general_purpose_registers[i] = memory[idx_reg + i];
                    }
                    break;
                }
                
                
                default:
                    std::cout << "Unimplemented FX__ opcode: 0x"
                              << std::hex << std::setfill('0') << std::setw(4)
                              << instruction << std::dec << std::endl;
                    break;
            }
            break;
        }
        case 0xC: {
            /*
            This instruction generates a random number, binary ANDs it with
            the value NN, and puts the result in VX.
            */

            /**
             * todo: WE'RE CREATING A RANDOM SEED EVERY TIME, SO IT MIGHT BE A GOOD
             * IDEA TO CONSTRUCT IT SOMEWHERE.
             */
            
            std::random_device rd;

            std::mt19937 gen(rd());

            std::uniform_int_distribution<int> distrib(0, 255);

            general_purpose_registers[x] = distrib(gen) & nn;
            break;   
        }
        case 0xB: {
            /*
             this instruction jumped to the address NNN plus the value in the
             register V0. This was mainly used for “jump tables”, to quickly be able to jump to different subroutines based on some input.
            */
            PC = nnn + general_purpose_registers[0];
            break;
        }
        case 0xE: {
            switch (nn) {
                case 0x9E: {

                    /**
                     * EX9E will skip one instruction (increment PC by 2) if the
                     * key corresponding to the value in VX is pressed.
                     */
                    // & 0x0F: only 16 keys exist, and VX can hold any byte.
                    // Real hardware decoded 4 bits, so wrap rather than read
                    // past the end of the array.
                    if (keypad[general_purpose_registers[x] & 0x0F]) {
                        PC += 2;
                    }
                    break;
                }
                case 0xA1: {
                    /**
                     *  skips if the key corresponding to the value in VX is not pressed.
                     */
                    if (!keypad[general_purpose_registers[x] & 0x0F]) {
                        PC += 2;
                    }
                    break;
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