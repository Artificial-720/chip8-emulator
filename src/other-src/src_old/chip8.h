#ifndef CHIP8
#define CHIP8

#include <stdint.h>

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define CHAR_COLORED "\u2588"
#define MAX_PROGRAM_SIZE 0xDFF
#define KEY_0 0x78
#define KEY_1 0x31
#define KEY_2 0x32
#define KEY_3 0x33
#define KEY_4 0x71
#define KEY_5 0x77
#define KEY_6 0x65
#define KEY_7 0x61
#define KEY_8 0x73
#define KEY_9 0x64
#define KEY_A 0x7A
#define KEY_B 0x63
#define KEY_C 0x34
#define KEY_D 0x72
#define KEY_E 0x66
#define KEY_F 0x76


struct Chip8 {
  // display 64x32 pixel monocrome display
  uint64_t pixels[DISPLAY_HEIGHT];

  uint8_t registers[0x10];  // 16 general purpose 8-bit registers
  uint16_t i;  // 16-bit register called i for index, used with the draw command
  uint16_t sp;  // stack pointer
  uint16_t pc;  // program counter
  uint8_t dt;   // delay timer
  uint8_t st;   // sound timer

  // memory 4KB (4096 bytes)
  uint8_t memory[4096];
  uint8_t keys[0x10];
};

// Function definitions
void read_file_contents(char *filename, uint8_t *buffer);
void execute(struct Chip8 *chip8);
void display(uint64_t *pixels, int size);
void execute_opcode(struct Chip8 *chip8, uint16_t opcode);
void print_bin(uint64_t num);
void clear_display(uint64_t *pixels, int size);
void draw(struct Chip8 *chip8, uint8_t x, uint8_t y, uint8_t *sprite,
          uint8_t size);
int convert_asscii_keycode(uint8_t c, uint8_t *converted);
void change_key_state(struct Chip8 *chip8, uint8_t asscii_key, uint8_t state);


void init_chip8(struct Chip8 *chip8);


void chip8_init(struct Chip8 *chip8);
void chip8_execute(struct Chip8 *chip8);


#endif