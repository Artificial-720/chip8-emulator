#include "chip8.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define PROGRAM_START_ADDRESS 0x200
#define MAX_PROGRAM_SIZE 0xDFF
#define SPRITE_WIDTH 8

static void clear_screen(Chip8* chip);
static void execute(Chip8* chip, uint16_t opcode);
static void instructions_draw_sprite(Chip8* chip, uint8_t x, uint8_t y, uint8_t n);
static void instructions_compare(Chip8* chip, uint8_t x, uint8_t y, uint8_t n);
static void instructions_f_branch(Chip8* chip, uint8_t x, uint8_t kk);


void chip8_init(Chip8* chip) {
  assert(chip);

  // Zero out all memory
  chip->SP = 0;
  chip->DT = 0;
  chip->ST = 0;
  chip->I = 0;
  chip->PC = PROGRAM_START_ADDRESS;
  for (int i = 0; i < REGISTERS_SIZE; i++) {
    chip->registers[i] = 0;
  }
  for (int i = 0; i < STACK_SIZE; i++) {
    chip->stack[i] = 0;
  }
  for (int i = 0; i < MEMORY_SIZE; i++) {
    chip->memory[i] = 0;
  }
  for (int i = 0; i < KEYS_SIZE; i++) {
    chip->keys[i] = 0;
    chip->keys_memory[i] = 0;
  }
  clear_screen(chip);

  // Load fonts into memory (0x000 to 0x1FF)
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
  for (long unsigned int i = 0; i < sizeof(fonts); i++) {
    chip->memory[i] = fonts[i];
  }
  // Seed random number
  srand(time(NULL));
}

int chip8_load_file(Chip8* chip, const char* filename) {
  assert(chip);
  assert(filename);

  uint8_t* buffer = chip->memory + PROGRAM_START_ADDRESS;
  FILE* fp;
  int file_size = 0;
  fp = fopen(filename, "rb");
  if (!fp) {
    return 1;
  }

  fseek(fp, 0, SEEK_END);
  file_size = ftell(fp);
  rewind(fp);

  if (file_size >= MAX_PROGRAM_SIZE) {
    fclose(fp);
    return 2;
  }
  fread(buffer, file_size, 1, fp);

  fclose(fp);
  return 0;
}

void chip8_timer_tick(Chip8* chip) {
  // Timers when non-zero, decremented at rate of 60Hz
  assert(chip);

  if (chip->DT) {
    chip->DT = chip->DT - 1;
  }
  if (chip->ST) {
    chip->ST = chip->ST - 1;
    if (chip->ST == 0) {
      printf("\aBEEP!\n");
    }
  }
}

void chip8_step(Chip8* chip) {
  assert(chip);
  // Fetch
  // instructions are stored big-endian
  // Read memory at PC
  uint16_t opcode;
  opcode = chip->memory[chip->PC]; // Upper byte
  opcode = opcode << 8;
  opcode = opcode | chip->memory[chip->PC + 1]; // Lower byte

  // PC incremented to next instruction
  chip->PC = chip->PC + 2;

  // Decode and Execute
  execute(chip, opcode);
}

// ----------------------------------------------------------------------------
// Static functions
// ----------------------------------------------------------------------------

static void execute(Chip8* chip, uint16_t opcode) {
  // 00E0 - CLS
  if (opcode == 0x00E0) {
    clear_screen(chip);
    return;
  }
  // 00EE - RET
  if (opcode == 0x00EE) {
    uint16_t sp = chip->SP;
    assert(sp < STACK_SIZE);
    chip->PC = chip->stack[sp];
    chip->SP = sp - 1;
    return;
  }

  // Decode
  // 0nnn 000n 0x00 00y0 00kk
  uint8_t x = (uint8_t)((opcode & 0x0F00) >> 8);
  uint8_t y = (uint8_t)((opcode & 0x00F0) >> 4);
  uint8_t kk = (uint8_t)(opcode & 0x00FF);
  uint8_t n = (uint8_t)(opcode & 0x000F);
  uint16_t nnn = opcode & 0x0FFF;

  // Execute
  switch (opcode & 0xF000) {
    case 0x1000:
      // 1nnn - JP addr
      chip->PC = nnn;
      break;
    case 0x2000:
      // 2nnn - CALL addr
      chip->SP += 1;
      chip->stack[chip->SP] = chip->PC;
      chip->PC = nnn;
      break;
    case 0x3000:
      // 3xkk - SE Vx, byte
      if (chip->registers[x] == kk) {
        chip->PC += 2;
      }
      break;
    case 0x4000:
      // 4xkk - SNE Vx, byte
      if (chip->registers[x] != kk) {
        chip->PC += 2;
      }
      break;
    case 0x5000:
      // 4xkk - SNE Vx, byte
      if (chip->registers[x] == chip->registers[y]) {
        chip->PC += 2;
      }
      break;
    case 0x6000:
      // 6xkk - LD Vx, byte
      chip->registers[x] = kk;
      break;
    case 0x7000:
      // 7xkk - ADD Vx, byte
      chip->registers[x] = chip->registers[x] + kk;
      break;
    case 0x8000:
      // Compare
      instructions_compare(chip, x, y, n);
      break;
    case 0x9000:
      // 9xy0 - SNE Vx, Vy
      if (chip->registers[x] != chip->registers[y]) {
        chip->PC += 2;
      }
      break;
    case 0xA000:
      // Annn - LD I, addr
      chip->I = nnn;
      break;
    case 0xB000:
      // Bnnn - JP V0, addr
      chip->PC = chip->registers[0] + nnn;
      break;
    case 0xC000:
      // Cxkk - RND Vx, byte
      chip->registers[x] = (rand() % 256) & kk;
      break;
    case 0xD000:
      // Dxyn - DRW Vx, Vy, nibble
      instructions_draw_sprite(chip, x, y, n);
      break;
    case 0xE000:
      // Ex9E - SKP Vx
      // ExA1 - SKNP Vx
      if (kk == 0x9E) {
        if (chip->keys[chip->registers[x]]) {
          chip->PC += 2;
        }
      } else if (kk == 0xA1) {
        if (!chip->keys[chip->registers[x]]) {
          chip->PC += 2;
        }
      }
      break;
    case 0xF000:
      instructions_f_branch(chip, x, kk);
      break;
    default:
      printf("Unknown opcode.");
      break;
  }
}

static void instructions_f_branch(Chip8* chip, uint8_t x, uint8_t kk) {
  uint8_t temp = 0;

  switch (kk) {
    case 0x07: // Fx07 - LD Vx, DT
      chip->registers[x] = chip->DT;
      break;
    case 0x0A: // Fx0A - LD Vx, K
      for (int i = 0; i < KEYS_SIZE; i++) {
        if (chip->keys[i]) {
          chip->keys_memory[i] = 1; // pressed
        } else if (chip->keys_memory[i]) {
          chip->registers[x] = i; // key released that was pressed
          temp = 1;
          // zero out memory
          for (int j = 0; j < KEYS_SIZE; j++) {
            chip->keys_memory[j] = 0;
          }
          break;
        }
      }
      if (!temp) { // back up ie block until key press
        chip->PC -= 2;
      }
      break;
    case 0x15: // Fx15 - LD DT, Vx
      chip->DT = chip->registers[x];
      break;
    case 0x18: // Fx18 - LD ST, Vx
      chip->ST = chip->registers[x];
      break;
    case 0x1E: // Fx1E - ADD I, Vx
      chip->I = chip->I + chip->registers[x];
      break;
    case 0x29: // Fx29 - LD F, Vx
      chip->I = 5 * chip->registers[x];
      break;
    case 0x33: // Fx33 - LD B, Vx
      chip->memory[chip->I] = chip->registers[x] / 100; // 100s
      chip->memory[chip->I + 1] = (chip->registers[x] % 100) / 10; // 10s
      chip->memory[chip->I + 2] = chip->registers[x] % 10;// 1s
      break;
    case 0x55: // Fx55 - LD [I], Vx
      for (int i = 0; i <= x; i++) {
        chip->memory[chip->I] = chip->registers[i];
        chip->I = chip->I + 1;
      }
      break;
    case 0x65: // Fx65 - LD Vx, [I]
      for (int i = 0; i <= x; i++) {
        chip->registers[i] = chip->memory[chip->I];
        chip->I = chip->I + 1;
      }
      break;
    default:
      printf("Unknown opcode.");
      break;
  }
}

static void instructions_compare(Chip8* chip, uint8_t x, uint8_t y, uint8_t n) {
  uint16_t temp = 0;

  switch (n) {
    case 0x00: // 8xy0 - LD Vx, Vy
      chip->registers[x] = chip->registers[y];
      break;
    case 0x01: // 8xy1 - OR Vx, Vy
      chip->registers[x] = chip->registers[x] | chip->registers[y];
      chip->registers[0xF] = 0;
      break;
    case 0x02: // 8xy2 - AND Vx, Vy
      chip->registers[x] = chip->registers[x] & chip->registers[y];
      chip->registers[0xF] = 0;
      break;
    case 0x03: // 8xy3 - XOR Vx, Vy
      chip->registers[x] = chip->registers[x] ^ chip->registers[y];
      chip->registers[0xF] = 0;
      break;
    case 0x04: // 8xy4 - ADD Vx, Vy
      temp = (uint16_t)chip->registers[x] + chip->registers[y];
      chip->registers[x] = (uint8_t)temp;
      chip->registers[0xF] = (temp > 255) ? 1 : 0;
      break;
    case 0x05: // 8xy5 - SUB Vx, Vy
      temp = (chip->registers[x] >= chip->registers[y]) ? 1 : 0;
      chip->registers[x] = chip->registers[x] - chip->registers[y];
      chip->registers[0xF] = temp;
      break;
    case 0x06: // 8xy6 - SHR Vx {, Vy}
      chip->registers[x] = chip->registers[y];
      temp = chip->registers[x] & 0x1;
      chip->registers[x] = chip->registers[x] >> 1;
      chip->registers[0xF] = temp;
      break;
    case 0x07: // 8xy7 - SUBN Vx, Vy
      temp = (chip->registers[y] >= chip->registers[x]) ? 1 : 0;
      chip->registers[x] = chip->registers[y] - chip->registers[x];
      chip->registers[0xF] = temp;
      break;
    case 0x0E: // 8xyE - SHL Vx {, Vy}
      chip->registers[x] = chip->registers[y];
      temp = (chip->registers[x] & 0x80) ? 1 : 0;
      chip->registers[x] = chip->registers[x] << 1;
      chip->registers[0xF] = temp;
      break;
    default:
      printf("Unknown opcode.");
      break;
  }
}



static void instructions_draw_sprite(Chip8* chip, uint8_t x, uint8_t y, uint8_t n) {
  uint8_t sprite_row;
  uint16_t sprite_address = chip->I;
  uint8_t x_coord = chip->registers[x] % DISPLAY_WIDTH;
  uint8_t y_coord = chip->registers[y] % DISPLAY_HEGIHT;
  uint64_t bit = 0;
  uint64_t mask = 0;

  chip->registers[0xF] = 0;

  for (int i = 0; i < n; i++) {
    if (y_coord + i >= DISPLAY_HEGIHT) {
      break;
    }
    sprite_row = chip->memory[sprite_address + i];
    for (int j = 0; j < SPRITE_WIDTH; j++) {
      if (x_coord + j >= DISPLAY_WIDTH) {
        break;
      }
      bit = sprite_row >> (SPRITE_WIDTH - 1 - j);
      bit = bit & 0x1;
      bit = bit << (63 - (x_coord + j));
      mask = 0x1UL << (63 - (x_coord + j));

      // if both bit and pixel are set then set VF
      if (bit && (mask & chip->pixels[y_coord + i]) && !chip->registers[0xF]) {
        chip->registers[0xF] = 1;
      }
      // xor in the bit
      chip->pixels[y_coord + i] = chip->pixels[y_coord + i] ^ bit;
    }
  }

  chip->draw_flag = 1;
}



static void clear_screen(Chip8* chip) {
  assert(chip);
  for (int i = 0; i < PIXELS_SIZE; i++) {
    chip->pixels[i] = 0UL;
  }
  chip->draw_flag = 1;
}
