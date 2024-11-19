#include <stdio.h>
#include "chip8.h"

int main(){
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


int cls(struct Chip8 *Chip8){
  
}