#include <stdint.h>  // uint8_t
#include <stdio.h>

#define PIXEL_SIZE 32
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

/*
Input
Finish implementing all opcodes
Hook up opengl


use this command to convert hex textfile into binary
xxd -r -p input.txt output.bin
*/

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

// function declerations
void print_bin(uint64_t num);
void display(uint64_t *pixels, int size);
void clear_display(uint64_t *pixels, int size);
void execute(struct Chip8 *chip8);
void load_prog_memory(uint8_t *memory, uint8_t *program, int prog_size);
void execute_opcode(struct Chip8 *chip8, uint16_t opcode);
void draw(struct Chip8 *chip8, uint8_t x, uint8_t y, uint8_t *sprite,
          uint8_t size);
void init_chip8(struct Chip8 *chip8);
void read_file_contents(char *filename, uint8_t *buffer);
int convert_asscii_keycode(uint8_t c, uint8_t *converted);

int main(int argc, char *argv[]) {
  // catch wrong args given
  if (argc > 2) {
    printf("Too many args.\n");
    return 0;
  } else if (argc < 2) {
    printf("One arg expected.\n");
    return 0;
  }
  // uint8_t code[] = {0x00, 0xE0, 0x60, 0x00, 0x61, 0x00, 0xa0,
  //                   0x0a, 0xd0, 0x15};
  struct Chip8 chip8;  // create chip
  init_chip8(&chip8);  // init chip
  uint8_t *mem_start = chip8.memory + 0x200;
  read_file_contents(argv[1], mem_start);  // load prog into memory
  for (int i = 0x200; i < 0x200 + 50; i++) {
    printf("%04X : %02X\n", i, chip8.memory[i]);
  }
  // start loop
  // while true:
  //   clock tick
  //   handle keyboard input
  //   sound timer beep?
  //   execute next instuction
  //   display screen
  // for (int i = 0; i < 7; i++) {
  while (1) {
    printf("loop press enter to continue to next execute\n");
    printf("V0: %02X  V1: %02X\n", chip8.registers[0], chip8.registers[1]);
    printf("i: %04X\n\n", chip8.i);
    // getchar();
    execute(&chip8);
    display(chip8.pixels, PIXEL_SIZE);
  }
  return 0;
}

void read_file_contents(char *filename, uint8_t *buffer) {
  if (!buffer || !filename) return;
  FILE *fp;
  long size;

  fp = fopen(filename, "rb");
  if (fp) {
    fseek(fp, 0L, SEEK_END);  // seek to end of file
    size = ftell(fp);         // tell how long file is
    rewind(fp);               // move back to start
    if (size <= MAX_PROGRAM_SIZE) fread(buffer, size, 1, fp);
    fclose(fp);
  }
}

void init_chip8(struct Chip8 *chip8) {
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
}

void print_bin(uint64_t num) {
  int i;
  for (i = 63; i >= 0; i--) {
    if (num & (1ULL << i)) {
      printf("\u2588");
    } else {
      printf(" ");
    }
  }
}

void display(uint64_t *pixels, int size) {
  int i;
  for (i = 0; i < size; i++) {
    printf("|");
    print_bin(pixels[i]);
    printf("|\n");
  }
}

// sets all to zero
void clear_display(uint64_t *pixels, int size) {
  // memset(pixels, 0, size * sizeof(*pixels));
  int i;
  for (i = 0; i < size; i++) {
    pixels[i] = 0;
  }
}

void load_prog_memory(uint8_t *memory, uint8_t *program, int prog_size) {
  int i;
  int offset = 0x200;
  uint8_t *mem_pointer = memory + offset;
  uint8_t *pro_pointer = program;

  for (i = 0; i < prog_size; i++) {
    *mem_pointer = *pro_pointer;
    mem_pointer++;
    pro_pointer++;
  }
}

void execute(struct Chip8 *chip8) {
  printf("execute\n");
  int index = chip8->pc;
  printf("index: %d\n", index);
  uint8_t high = chip8->memory[index];
  uint8_t low = chip8->memory[index + 1];
  printf("high: %02x\n", high);
  printf("low: %02x\n", low);
  uint16_t opcode = (high << 8) | low;
  printf("opcode: %04x\n", opcode);
  chip8->pc = index + 2;
  execute_opcode(chip8, opcode);
}

void execute_opcode(struct Chip8 *chip8, uint16_t opcode) {
  printf("execute_opcode: %04X\n", opcode);

  uint8_t x, y, kk, nnn, n;
  // 0x00, 00y0, 00kk, 0nnn, 000n
  x = (opcode & 0x0f00) >> 8;
  y = (opcode & 0x00f0) >> 4;
  kk = opcode & 0x00ff;
  nnn = opcode & 0x0fff;
  n = opcode & 0x000f;
  printf("x:%x y:%x kk:%x nnn:%x", x, y, kk, nnn);

  char c;

  switch (opcode & 0xF000) {
    case 0x0000:
      printf("0x0000\n");
      // 00e0 - clear display
      // 00ee - set pc to address at top of stack, subtracts 1 from stack
      // pointer
      if (opcode == 0x00e0)
        clear_display(chip8->pixels, PIXEL_SIZE);
      else if (opcode == 0x00ee)
        printf("todo\n");  // todo
      break;
    case 0x1000:
      printf("0x1000\n");
      chip8->pc = opcode & 0x0fff;
      break;
    case 0x2000:
      printf("0x2000\n");
      // 2nnn - call subroutine at nnn
      // increment sp, put current pc on top of stack, pc set to nnn
      chip8->sp++;
      // todo current pc on top of stack
      chip8->pc = opcode & 0x0fff;
      break;
    case 0x3000:
      printf("0x3000\n");
      // 3xkk - skip next instruction if Vx = kk
      if (chip8->registers[x] == kk) chip8->pc += 2;
      break;
    case 0x4000:
      printf("0x4000\n");
      // 4xkk - skip next instruction if Vx != kk
      if (chip8->registers[x] != kk) chip8->pc += 2;
      break;
    case 0x5000:
      printf("0x5000\n");
      // 5xy0 - skip next instruction if Vx = Vy
      if (chip8->registers[x] == chip8->registers[y]) chip8->pc += 2;
      break;
    case 0x6000:
      printf("0x6000\n");
      // 6xkk - put value kk into register Vx
      chip8->registers[x] = kk;
      break;
    case 0x7000:
      printf("0x7000\n");
      // 7xkk - add value kk to register Vx, then store in Vx
      chip8->registers[x] += kk;
      break;
    case 0x8000:
      printf("0x8000\n");
      // 8xy0 - store the value of register Vy in register Vx
      chip8->registers[x] = chip8->registers[y];
      // todo
      // 8xy1 - bitwise OR on values of Vx and Vy, stores the result in Vx
      // 8xy2 - bitwise AND on Vx and Vy, stores result in Vx
      // 8xy3 - bitwise XOR on Vx and Vy, stores result in Vx
      // 8xy4 - Set Vx = Vx + Vy, set VF = carry. result > 8bits set carry
      // 8xy5 - Set Vx = Vx - Vy, set VF = NOT borrow.
      // 8xy6 - Store least significant bit of Vx in VF, then shift right
      // 8xy7 - Set Vx = Vy - Vx, set VF = NOT borrow.
      // 8xyE - Store most significant bit of Vx in VF, then shift left
      break;
    case 0x9000:
      printf("0x9000\n");
      // 9xy0 - skip next instruction if Vx != Vy
      if (chip8->registers[x] != chip8->registers[y]) chip8->pc += 2;
      break;
    case 0xa000:
      printf("0xa000\n");
      // annn - set register I = nnn
      chip8->i = nnn;
      break;
    case 0xb000:
      printf("0xb000\n");
      // bnnn - pc = nnn + V0
      chip8->pc = nnn + chip8->registers[0];
      break;
    case 0xc000:
      printf("0xc000\n");
      // cxkk - Vx = random byte AND kk
      // todo
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
      draw(chip8, chip8->registers[x], chip8->registers[y],
           &(chip8->memory[chip8->i]), n);

      break;
    case 0xe000:
      printf("0xe000\n");
      // ex9e - skip next instruction if key with value of Vx is pressed
      // todo
      // exa1 - skip next instruction if key with value of Vx is not pressed
      // todo
      break;
    case 0xf000:
      printf("0xf000\n");
      switch (opcode & 0x00ff) {
        case 0x07:
          // fx07 - Vx = delay timer value
          chip8->registers[x] = chip8->dt;
          break;

        case 0x0a:
          // fx0a - wait for key press, store value in Vx
          //        all execution stops until a key is pressed
          // todo
          // printf("Enter character: ");
          // c = getchar();
          // printf("Character entered: ");
          // putchar(c);
          do{
            printf("Press a key: ");
          }while(convert_asscii_keycode(getchar(), &c));
          chip8->registers[x] = (uint8_t)c;
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
          // todo
          break;
        case 0x65:
          // fx65 - read registers V0 through Vx from memory starting at I
          // todo
          break;
        default:
          break;
      }
      break;

    default:
      break;
  }
}

void draw(struct Chip8 *chip8, uint8_t x, uint8_t y, uint8_t *sprite,
          uint8_t size) {
  // printf("draw\n x:%02x y:%02x\n sprite:%04x size: %02x\n", x, y, sprite,
  // size); x range 0-63 x coordinate y range 0-31 y coordinate n range 0-15
  // number of bytes
  int i;
  uint64_t shifted;
  size = size & 0xF;  // prevent out of bounds
  for (i = 0; i < size; i++) {
    shifted = (uint64_t)(sprite[i]);
    printf("1 shift: %016lX\n", shifted);
    shifted = shifted << (56 - x);
    printf("2 shift: %016lX\n", shifted);
    chip8->pixels[i + y] = chip8->pixels[i + y] ^ shifted;
    printf("hex: %016lX\n", chip8->pixels[i + y]);
  }
}

int convert_asscii_keycode(uint8_t c, uint8_t *converted) {
  // Return 1 on fail
  switch (c) {
    case KEY_0:
      *converted = 0x0;
      break;
    case KEY_1:
      *converted = 0x1;
      break;
    case KEY_2:
      *converted = 0x2;
      break;
    case KEY_3:
      *converted = 0x3;
      break;
    case KEY_4:
      *converted = 0x4;
      break;
    case KEY_5:
      *converted = 0x5;
      break;
    case KEY_6:
      *converted = 0x6;
      break;
    case KEY_7:
      *converted = 0x7;
      break;
    case KEY_8:
      *converted = 0x8;
      break;
    case KEY_9:
      *converted = 0x9;
      break;
    case KEY_A:
      *converted = 0xA;
      break;
    case KEY_B:
      *converted = 0xB;
      break;
    case KEY_C:
      *converted = 0xC;
      break;
    case KEY_D:
      *converted = 0xD;
      break;
    case KEY_E:
      *converted = 0xE;
      break;
    case KEY_F:
      *converted = 0xF;
      break;
    default:
      return 1;
      break;
  }
  return 0;
}