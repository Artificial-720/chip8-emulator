#ifndef CHIP8
#define CHIP8

#include <stdint.h>

struct Chip8 {
  // display 64x32 pixel monocrome display
  uint64_t pixels[PIXEL_SIZE];

  uint8_t registers[0x10];  // 16 general purpose 8-bit registers
  uint16_t i;  // 16-bit register called i for index, used with the draw command
  uint16_t sp;  // stack pointer
  uint16_t pc;  // program counter
  uint8_t dt;   // delay timer
  uint8_t st;   // sound timer

  // memory 4KB (4096 bytes)
  uint8_t memory[4096];
};

#endif