#include <array>

#include "frontend.h"
#include "chip8.h"
#include "raylib.h"

/**
 * How many screen pixels each CHIP-8 pixel takes up, in each direction.
 * Used for both the window size and the drawing in draw(), so the image
 * always fills the window exactly.
 */
constexpr int SCALE = 10;

/**
 * Keyboard key for each CHIP-8 key, indexed by CHIP-8 key number (0x0-0xF).
 * So KEYMAP[0x4] is the key that acts as CHIP-8 key 4.
 *
 *   Keyboard        CHIP-8
 *   1 2 3 4         1 2 3 C
 *   Q W E R   ->    4 5 6 D
 *   A S D F         7 8 9 E
 *   Z X C V         A 0 B F
 */
static constexpr std::array<KeyboardKey, 16> KEYMAP = {
    KEY_X,     // 0
    KEY_ONE,   // 1
    KEY_TWO,   // 2
    KEY_THREE, // 3
    KEY_Q,     // 4
    KEY_W,     // 5
    KEY_E,     // 6
    KEY_A,     // 7
    KEY_S,     // 8
    KEY_D,     // 9
    KEY_Z,     // A
    KEY_C,     // B
    KEY_FOUR,  // C
    KEY_R,     // D
    KEY_F,     // E
    KEY_V,     // F
};

void Frontend::init() {
    // 64 x 32 CHIP-8 pixels, each drawn as a SCALE x SCALE square
    InitWindow(64 * SCALE, 32 * SCALE, "Chip8 Title");

    SetTargetFPS(60);

    InitAudioDevice();
}

bool Frontend::should_close() {
    // True once the user clicks the window's close button or presses Escape.
    return WindowShouldClose();
}

void Frontend::shutdown() {
    // Tear down in the reverse order of init().
    CloseAudioDevice();
    CloseWindow();
}

void Frontend::read_keys(Chip8& chip8) {
    // Polling: once per frame, ask whether each key is held right now and
    // copy that into the keypad. k is the CHIP-8 key number (0x0-0xF);
    // KEYMAP[k] is the keyboard key standing in for it.
    for (int k = 0; k < 16; k++) {
        chip8.keypad[k] = IsKeyDown(KEYMAP[k]);
    }
}

void Frontend::draw(const Chip8& chip8) {
    BeginDrawing();

    ClearBackground(BLACK);

    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 64; j++) {
            if (chip8.display[i][j]) {
                // x comes from the column (j), y from the row (i)
                DrawRectangle(j * SCALE, i * SCALE, SCALE, SCALE, WHITE);
            }
        }
    }

    EndDrawing();
}