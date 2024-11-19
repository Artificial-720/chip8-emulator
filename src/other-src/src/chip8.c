#include "chip8.h"

#include <stdio.h>

// runs the execute step
static void chip8_execute_opcode_internal(struct Chip8 *chip8, uint16_t opcode);
// draws sprite into pixels
static int chip8_draw_internal(struct Chip8 *chip8, uint8_t x, uint8_t y,
                               uint8_t *sprite, uint8_t sprite_size);

void chip8_init(struct Chip8 *chip8) {
  chip8->pc = 0x200;
  uint8_t fonts[] = {
      0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
      0x20, 0x60, 0x20, 0x20, 0x70,  // 1
      0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
      0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
      0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
      0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
      0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
      0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
      0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
      0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
      0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
      0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
      0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
      0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
      0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
      0xF0, 0x80, 0xF0, 0x80, 0x80   // F
  };
  for (unsigned long i = 0; i < sizeof(fonts); i++) {
    chip8->memory[i] = fonts[i];
  }
  for (int i = 0; i < 16; i++) {
    chip8->keys[i] = 0;
  }
}

void chip8_execute(struct Chip8 *chip8) {
  int index = chip8->pc;
  uint8_t high = chip8->memory[index];
  uint8_t low = chip8->memory[index + 1];
  uint16_t opcode = (high << 8) | low;
  printf("opcode: %04x\n", opcode);
  chip8->pc = index + 2;
  chip8_execute_opcode_internal(chip8, opcode);
}

int chip8_file_into_memory(struct Chip8 *chip8, char *filename) {
  uint8_t *buffer = chip8->memory + PROGRAM_START_ADDRESS;
  if (!buffer || !filename) return 0;
  FILE *fp;
  long size;

  fp = fopen(filename, "rb");
  if (fp) {
    fseek(fp, 0L, SEEK_END);  // seek to end of file
    size = ftell(fp);         // tell how long file is
    rewind(fp);               // move back to start
    if (size <= MAX_PROGRAM_SIZE) fread(buffer, size, 1, fp);
    fclose(fp);
  } else
    return 0;
  return 1;
}

void chip8_key_state(struct Chip8 *chip8, uint8_t asscii_key, uint8_t state) {
  
}

void chip8_std_display(struct Chip8 *chip8) {}

static int chip8_draw_internal(struct Chip8 *chip8, uint8_t x, uint8_t y,
                               uint8_t *sprite, uint8_t sprite_size) {}

static void chip8_execute_opcode_internal(struct Chip8 *chip8,
                                          uint16_t opcode) {}