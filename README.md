# Chip-8 Emulator

Implementation of the classic chip-8 in C using OpenGL for graphics rendering.

## Chip-8 technical reference

http://devernay.free.fr/hacks/chip8/C8TECH10.HTM

This guide was also helpful:

https://tobiasvl.github.io/blog/write-a-chip-8-emulator/


## Tests

This emulator was tested against Timendus's test suite and passes all tests.
https://github.com/Timendus/chip8-test-suite


## Keybindings

F11 - Toggle full screen
Esc - Exit
B - Toggle debug
N - Step

```
CHIP-8 Key   Keyboard
---------   ---------
1 2 3 C     1 2 3 4
4 5 6 D     Q W E R
7 8 9 E     A S D F
A 0 B F     Z X C V
```

## Install and Usage

### Clone

```bash
git clone https://github.com/Artificial-720/chip8-emulator/tree/main
cd chip8-emulator
```

### Build

```bash
make
```

### Run

```bash
./chip8 <rom filename>
```
