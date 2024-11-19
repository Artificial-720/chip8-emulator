#ifndef CHIP8
#define CHIP8

#include <stdint.h>

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define CHAR_COLORED "\u2588"
#define PROGRAM_START_ADDRESS 0x200
#define MAX_PROGRAM_SIZE 0xDFF
#define KEY_0 0x58 // X
#define KEY_1 0x31 // 1
#define KEY_2 0x32 // 2
#define KEY_3 0x33 // 3
#define KEY_4 0x51 // Q
#define KEY_5 0x57 // W
#define KEY_6 0x45 // E
#define KEY_7 0x41 // A
#define KEY_8 0x53 // S
#define KEY_9 0x44 // D
#define KEY_A 0x5A // Z
#define KEY_B 0x43 // C
#define KEY_C 0x34 // 4
#define KEY_D 0x52 // R
#define KEY_E 0x46 // F
#define KEY_F 0x56 // V

struct Chip8 {
  uint64_t pixels[DISPLAY_HEIGHT];  // display 64x32 pixel monocrome display
  uint8_t registers[0x10];          // 16 general purpose 8-bit registers
  uint16_t i;  // 16-bit register called i for index, used with the draw command
  uint16_t sp;           // stack pointer
  uint16_t pc;           // program counter
  uint8_t dt;            // delay timer
  uint8_t st;            // sound timer
  uint8_t memory[4096];  // memory 4KB (4096 bytes)
  uint8_t keys[0x10];    // state of the keyboard
  uint16_t stack[0x18];  // 48 bytes for stack
};

// Function definitions
void chip8_init(struct Chip8 *chip8); // sets inital values
void chip8_execute(struct Chip8 *chip8); // runs fetch decode and execute
int chip8_file_into_memory(struct Chip8 *chip8, char *filename); // reads file contents into memory
void chip8_key_state(struct Chip8 *chip8, uint8_t asscii_key, uint8_t state); // updates key state
void chip8_std_display(struct Chip8 *chip8); // outputs display to stdout

#endif