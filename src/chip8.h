#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>

#define REGISTERS_SIZE 16
#define STACK_SIZE 16
#define PIXELS_SIZE 32
#define MEMORY_SIZE 4095
#define KEYS_SIZE 16

#define DISPLAY_WIDTH 64
#define DISPLAY_HEGIHT 32

typedef struct {
  uint8_t draw_flag; // Whether pixels have been changed
  uint8_t keys[KEYS_SIZE]; // Keyboard state
  uint8_t keys_memory[KEYS_SIZE]; // Keyboard state history, used for opcode Fx0A
  uint8_t registers[REGISTERS_SIZE]; // 16 general purpose 8-bit registers
  uint8_t SP; // stack pointer, top of stack
  uint8_t DT; // Delay timer
  uint8_t ST; // Sound timer, buzz sound when non-zero
  uint16_t I; // 16-bit register stores memory addresses (only lower 12 bits are used)
  uint16_t PC; // program counter, current executing address
  uint16_t stack[STACK_SIZE]; // return addresses
  uint64_t pixels[PIXELS_SIZE]; // Display
  uint8_t memory[MEMORY_SIZE];  // RAM
} Chip8;

void chip8_init(Chip8* chip);
void chip8_timer_tick(Chip8* chip);
void chip8_step(Chip8* chip);
int chip8_load_file(Chip8* chip, const char* filename);

#endif
