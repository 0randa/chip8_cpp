#pragma once

#include <cstdint>
#include <iomanip>
#include <iostream>


inline void print_hex_byte(uint8_t value) {
    std::cout << std::hex << std::setw(2) << std::setfill('0')
              << static_cast<int>(value) << std::dec;
}
