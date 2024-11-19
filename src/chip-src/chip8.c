#include "chip8.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// runs the execute step
static void chip8_execute_opcode_internal(struct Chip8 *chip8, uint16_t opcode);
// draws sprite into pixels
static int chip8_draw_internal(struct Chip8 *chip8, uint8_t x, uint8_t y,
                               uint8_t *sprite, uint8_t sprite_size);

void chip8_init(struct Chip8 *chip8) {
  chip8->sp = 0;
  chip8->pc = 0x200;
  // load fonts into memory
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
  // clear keys
  for (int i = 0; i < 16; i++) {
    chip8->keys[i] = 0;
  }
  // Seed the random number generator
  srand(time(NULL));
  // clear display
  for (int i = 0; i < DISPLAY_HEIGHT; i++) {
    chip8->pixels[i] = 0;
  }
}

void chip8_execute(struct Chip8 *chip8) {
  int index = chip8->pc;
  uint8_t high = chip8->memory[index];
  uint8_t low = chip8->memory[index + 1];
  uint16_t opcode = (high << 8) | low;
  printf("pc: %04x\n", index);
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

void chip8_key_state(struct Chip8 *chip8, uint8_t keycode, uint8_t state) {
  if(state){
    printf("key state pressed. keycode: %02X. state: %02X\n", keycode, state);
  }
  chip8->keys[keycode] = state;
}

void chip8_std_display(struct Chip8 *chip8) {
  int i, j;
  uint64_t row;
  for (i = 0; i < DISPLAY_HEIGHT; i++) {
    printf("|");
    row = chip8->pixels[i];
    for (j = 63; j >= 0; j--) {
      if (row & (1ULL << j)) {
        printf("\u2588");
      } else {
        printf(" ");
      }
    }
    printf("| %016lX\n", row);
  }
}

static int chip8_draw_internal(struct Chip8 *chip8, uint8_t x, uint8_t y,
                               uint8_t *sprite, uint8_t sprite_size) {
  chip8->registers[0xF] = 0;
  // int intersection = 0;
  uint8_t x_pos = x % DISPLAY_WIDTH;
  uint8_t y_pos = y % DISPLAY_HEIGHT;
  int i, j;
  uint8_t byte;
  uint64_t shifted;
  printf("start of draw_internal\n");
  printf("x_pos: %02X\ny_pos: %02X\n", x_pos, y_pos);
  for (i = 0; i < sprite_size; i++) {
    if (i + y_pos >= DISPLAY_HEIGHT) {
      break;
    }
    byte = sprite[i];
    for (j = 0; j < 8; j++) {
      if (j + x_pos >= DISPLAY_WIDTH) {
        break;
      }
      // draw pixel and check for intersection
      // uint64_t pixel = 0;
      // pixel = byte & (0x1 << (7 - j));
      shifted = byte >> (7 - j);
      shifted = shifted & 0x1;
      shifted = shifted << (63 - (j + x_pos));
      // shifted = byte << (63 - (j + x_pos));
      printf("byte: %02X\nshifted: %016lX\n", byte, shifted);
      chip8->pixels[i + y_pos] = chip8->pixels[i + y_pos] ^ shifted;
    }
  }
  return 0;

  // int i;
  // uint64_t shifted;
  // sprite_size = sprite_size & 0xF;  // prevent out of bounds
  // for (i = 0; i < sprite_size; i++) {
  //   shifted = (uint64_t)(sprite[i]);
  //   printf("1 shift: %016lX\n", shifted);
  //   shifted = shifted << (56 - x);
  //   printf("2 shift: %016lX\n", shifted);
  //   chip8->pixels[i + y] = chip8->pixels[i + y] ^ shifted;
  //   printf("hex: %016lX\n", chip8->pixels[i + y]);
  // }
  // return 0;  // todo check for intersections
}

static void chip8_execute_opcode_internal(struct Chip8 *chip8,
                                          uint16_t opcode) {
  printf("execute_opcode: %04X\n", opcode);

  uint16_t x, y, kk, nnn, n;
  // 0x00, 00y0, 00kk, 0nnn, 000n
  x = (opcode & 0x0f00) >> 8;
  y = (opcode & 0x00f0) >> 4;
  kk = opcode & 0x00ff;
  nnn = opcode & 0x0fff;
  n = opcode & 0x000f;
  printf("x:%x y:%x kk:%x nnn:%x\n", x, y, kk, nnn);

  int i;
  uint8_t *pI;
  uint16_t temp;
  // char c;

  switch (opcode & 0xF000) {
    case 0x0000:
      // 00e0 - clear display
      if (opcode == 0x00e0) {
        printf("00e0 - clear display\n");
        for (i = 0; i < DISPLAY_HEIGHT; i++) {
          chip8->pixels[i] = 0;
        }
      } else if (opcode == 0x00ee) {
        // 00ee - set pc to address at top of stack, subtracts 1 from sp
        printf("00ee - return\n");
        chip8->pc =
            chip8->stack[chip8->sp];  // todo check that this opcode works
        printf("pc retrieved: %04X\n", chip8->pc);
        chip8->sp--;
      }
      break;
    case 0x1000:
      // 1nnn Jump to location nnn
      chip8->pc = nnn;
      break;
    case 0x2000:
      // 2nnn - call subroutine at nnn
      // increment sp, put current pc on top of stack, pc set to nnn
      printf("2nnn - call subroutine at nnn\n");
      printf("pc stored: %04X\n", chip8->pc);
      printf("SP: %04X\n", chip8->sp);
      chip8->sp++;  // todo check that is opcode works
      chip8->stack[chip8->sp] = chip8->pc;
      chip8->pc = nnn;
      break;
    case 0x3000:
      // 3xkk - skip next instruction if Vx = kk
      if (chip8->registers[x] == kk) chip8->pc += 2;
      break;
    case 0x4000:
      // 4xkk - skip next instruction if Vx != kk
      if (chip8->registers[x] != kk) chip8->pc += 2;
      break;
    case 0x5000:
      // 5xy0 - skip next instruction if Vx = Vy
      if (chip8->registers[x] == chip8->registers[y]) chip8->pc += 2;
      break;
    case 0x6000:
      // 6xkk - put value kk into register Vx
      chip8->registers[x] = kk;
      break;
    case 0x7000:
      // 7xkk - add value kk to register Vx, then store in Vx
      chip8->registers[x] += kk;
      break;
    case 0x8000:
      switch (n) {
        case 0x0:
          // 8xy0 - store the value of register Vy in register Vx
          chip8->registers[x] = chip8->registers[y];
          break;
        case 0x1:
          // 8xy1 - bitwise OR on values of Vx and Vy, stores the result in Vx
          chip8->registers[x] = chip8->registers[x] | chip8->registers[y];
          break;
        case 0x2:
          // 8xy2 - bitwise AND on Vx and Vy, stores result in Vx
          chip8->registers[x] = chip8->registers[x] & chip8->registers[y];
          break;
        case 0x3:
          // 8xy3 - bitwise XOR on Vx and Vy, stores result in Vx
          chip8->registers[x] = chip8->registers[x] ^ chip8->registers[y];
          break;
        case 0x4:
          // 8xy4 - Set Vx = Vx + Vy, set VF = carry. result > 8bits set carry
          temp = chip8->registers[x] + chip8->registers[y];
          chip8->registers[x] = (uint8_t)temp;
          temp &= 0xFF00;
          if (temp)
            temp = 0x1;
          else
            temp = 0;
          chip8->registers[0xF] = temp;  // should be 1 or 0
          break;
        case 0x5:
          // 8xy5 - Set Vx = Vx - Vy, set VF = NOT borrow.
          if (chip8->registers[x] > chip8->registers[y])
            temp = 0x1;
          else
            temp = 0;
          chip8->registers[x] = chip8->registers[x] - chip8->registers[y];
          chip8->registers[0xF] = temp;
          break;
        case 0x6:
          // 8xy6 - Store least significant bit of Vx in VF, then shift right
          temp = chip8->registers[x] & 0x1;
          chip8->registers[x] = chip8->registers[x] >> 1;
          chip8->registers[0xF] = temp;
          break;
        case 0x7:
          // 8xy7 - Set Vx = Vy - Vx, set VF = NOT borrow.
          if (chip8->registers[y] > chip8->registers[x])
            temp = 0x1;
          else
            temp = 0;
          chip8->registers[x] = chip8->registers[y] - chip8->registers[x];
          chip8->registers[0xF] = temp;
          break;
        case 0xe:
          // 8xyE - Store most significant bit of Vx in VF, then shift left
          temp = chip8->registers[x] & 0x80;
          chip8->registers[x] = chip8->registers[x] << 1;
          chip8->registers[0xF] = temp;
          break;
        default:
          break;
      }
      break;
    case 0x9000:
      // 9xy0 - skip next instruction if Vx != Vy
      if (chip8->registers[x] != chip8->registers[y]) chip8->pc += 2;
      break;
    case 0xa000:
      // annn - set register I = nnn
      chip8->i = nnn;
      break;
    case 0xb000:
      // bnnn - pc = nnn + V0
      chip8->pc = nnn + chip8->registers[0];
      break;
    case 0xc000:
      // cxkk - Vx = random byte AND kk
      uint8_t randomByte = rand() & 0xFF;
      chip8->registers[x] = randomByte & kk;
      break;
    case 0xd000:
      printf("0xd000\n");
      // dxyn
      // x range 0-63 x coordinate
      // y range 0-31 y coordinate
      // n range 0-15 number of bytes
      // todo
      // void draw(struct Chip8 *chip8, uint8_t x, uint8_t y, uint8_t *sprite,
      // uint8_t size)
      chip8_draw_internal(chip8, chip8->registers[x], chip8->registers[y],
                          &(chip8->memory[chip8->i]), n);

      break;
    case 0xe000:
      printf("0xe000\n");
      if (kk == 0x9e) {
        // ex9e - skip next instruction if key with value of Vx is pressed
        printf("we are in here checking if the value of Vx is pressed\n");
        printf("value of x: %02X\n", x);
        printf("key value: %02X\n", chip8->keys[x]);
        if (chip8->keys[chip8->registers[x]]) chip8->pc += 2;
      } else if (kk == 0xa1) {
        // exa1 - skip next instruction if key with value of Vx is not pressed
        if (!chip8->keys[chip8->registers[x]]) chip8->pc += 2;
      }
      break;
    case 0xf000:
      switch (opcode & 0x00ff) {
        case 0x07:
          // fx07 - Vx = delay timer value
          chip8->registers[x] = chip8->dt;
          break;

        case 0x0a:
          // fx0a - wait for key press, store value in Vx
          //        all execution stops until a key is pressed
          // todo stop the timer until key is pressed
          temp = 1;
          for(i = 0; i < 16; i++){
            if(chip8->keys[i]){
              chip8->registers[x] = i;
              temp = 0;
              break;
            }
          }
          if(temp) chip8->pc -= 2;
          break;
        case 0x15:
          // fx15 - set delay timer to Vx
          chip8->dt = chip8->registers[x];
          break;
        case 0x18:
          // fx18 - set sound timer to Vx
          chip8->st = chip8->registers[x];
          break;
        case 0x1e:
          // fx1e - Set I = I + Vx
          chip8->i = chip8->i + chip8->registers[x];
          break;
        case 0x29:
          // fx29 - I = location of sprite for digit Vx
          chip8->i = ((uint16_t)chip8->registers[x]) * 5;
          break;
        case 0x33:
          // fx33 - Store BCD representation of Vx in locations I, I+1, I+2
          // todo
          break;
        case 0x55:
          // fx55 - store registers V0 through Vx in memory starting at I
          pI = &(chip8->memory[chip8->i]);
          for (i = 0; i <= x; i++) {
            *pI++ = chip8->registers[i];
          }
          break;
        case 0x65:
          // fx65 - read registers V0 through Vx from memory starting at I
          pI = &(chip8->memory[chip8->i]);
          for (i = 0; i <= x; i++) {
            chip8->registers[i] = *pI++;
          }
          break;
        default:
          break;
      }
      break;

    default:
      break;
  }
}