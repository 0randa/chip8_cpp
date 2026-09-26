#pragma once

// Forward declaration: the functions below only take Chip8 by reference,
// so the compiler just needs to know the type exists. frontend.cpp includes
// chip8.h itself, since that file actually reads the machine's fields.
struct Chip8;

struct Frontend {
    void init(); /** Opens the window, sets 60 FPS and starts the audio */
    bool should_close(); /** determines whether the user closed the window */

    void read_keys(Chip8& chip8); /** fill the keypad array from the keyboard */

    void draw(const Chip8& chip8); /** Draw display to the window */

    void beep(bool on); /** play or stop a tone */

    void shutdown(); /** Close audio and the window */
};
