# chip8_cpp

A CHIP-8 emulator built in C++.

## Requirements

- macOS
- A C++17 compiler
- raylib and pkg-config

```bash
brew install raylib pkg-config
```

## Building and running

Build the emulator:

```bash
make
```

Run a ROM:

```bash
./chip8 path/to/rom.ch8
```

Press Escape or close the window to quit.

## Controls

The CHIP-8 keypad is mapped onto the left side of the keyboard:

```
Keyboard        CHIP-8 keypad
1 2 3 4         1 2 3 C
Q W E R   ->    4 5 6 D
A S D F         7 8 9 E
Z X C V         A 0 B F
```
