#include "frontend.h"
#include "chip8.h"
#include "raylib.h"

/**
 * How many screen pixels each CHIP-8 pixel takes up, in each direction.
 * Used for both the window size and the drawing in draw(), so the image
 * always fills the window exactly.
 */
constexpr int SCALE = 10;

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