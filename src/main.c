#define _POSIX_C_SOURCE 199309L // Needed to include nanosleep

#include "graphics.h"
#include "chip8.h"

#include <GL/gl.h>
#include <bits/time.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <GLFW/glfw3.h>
#include <time.h>
#include <sys/time.h>


#define KEY_DEBUG 0x42 // B
#define KEY_STEP 0x4E  // N
#define KEY_FULLSCREEN 0x12C // F11
#define KEY_EXIT 0x100 // Esc

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

typedef struct {
  int fullscreen_lock; // prevent rapid change of full screen
  int debug_lock;
  int step_lock;
  int step; // when 1 should run a step, in debug mode
  int mode; // 1 debug, 0 normal
  int refresh_window; // Flag to refresh window
} State;

static void print_debug(Chip8* chip);
static void update_keyboard_input(GLFWwindow* window, Chip8* chip8, State* state);
static void update_window_viewport(GLFWwindow* window, int* width, int* height, State* state);
static void update_color_buffer(Chip8* chip, unsigned int vbo_color, unsigned short* color_buffer, int count);

long current_time_millis() {
  struct timeval tp;
  gettimeofday(&tp, NULL);
  return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}


int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    return 0;
  }

  int width = WINDOW_WIDTH;
  int height = WINDOW_HEIGHT;
  GLFWwindow* window = init_window(width, height, "Chip 8");
  if (!window) return -1;

  if (install_shaders()) {
    glfwDestroyWindow(window);
    glfwTerminate();
    return -1;
  }

  unsigned int vbo_pos, vbo_color, ibo;
  int draw_count = 0;
  draw_count = setup_buffers(&vbo_pos, &vbo_color, &ibo);
  int cb_count = DISPLAY_WIDTH * DISPLAY_HEGIHT * 4;
  unsigned short color_buffer[cb_count];
  for (int i = 0; i < cb_count; i++) {
    color_buffer[i] = 0;
  }

  // Setup CHIP-8
  Chip8 chip;
  chip8_init(&chip);
  if (chip8_load_file(&chip, argv[1])) {
    printf("Failed to load file: %s\n", argv[1]);
    glfwDestroyWindow(window);
    glfwTerminate();
    return -1;
  }

  State state = {
    .fullscreen_lock = 0,
    .debug_lock = 0,
    .step_lock = 0,
    .mode = 0,
    .refresh_window = 1
  };

  long time_per_frame = 1000 / 60; // 60Hz
  long accumulated_time = 0;
  long previous_time = current_time_millis();
  while(!glfwWindowShouldClose(window)) {
    long now = current_time_millis();
    long deltatime = now - previous_time; // in milliseconds
    previous_time = now;
    accumulated_time += deltatime;

    update_keyboard_input(window, &chip, &state);
    update_window_viewport(window, &width, &height, &state);

    // idk if this is better
    // it seems that the display of test 5 can be fixed by moving the step out
    // side the time check
    if (state.mode == 0) {
      if (accumulated_time >= time_per_frame) {
        chip8_timer_tick(&chip);
        accumulated_time -= time_per_frame;
      }
      chip8_step(&chip);
    } else if (state.step) {
      print_debug(&chip);
      chip8_timer_tick(&chip);
      chip8_step(&chip);
      state.step = 0;
      accumulated_time = 0;
    }

    if (chip.draw_flag) {
      update_color_buffer(&chip, vbo_color, color_buffer, cb_count);
      chip.draw_flag = 0;
      state.refresh_window = 1;
    }
    if (state.refresh_window) {
      glClear(GL_COLOR_BUFFER_BIT);
      glDrawElements(GL_TRIANGLES, draw_count, GL_UNSIGNED_SHORT, NULL);
      glfwSwapBuffers(window);
      state.refresh_window = 0;
    }

    glfwPollEvents();

    // Sleep to prevent CPU hog
    struct timespec req = {
      .tv_sec = 0,
      .tv_nsec = 1000000L
    };
    nanosleep(&req, NULL);
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}




static void update_keyboard_input(GLFWwindow* window, Chip8* chip, State* state) {
  assert(window);
  assert(chip);
  assert(state);

  int key_state = 0;

  // Check for escape key press
  key_state = glfwGetKey(window, KEY_EXIT);
  if (key_state == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }

  // Check for full screen toggle
  key_state = glfwGetKey(window, KEY_FULLSCREEN);
  if (key_state == GLFW_PRESS) {
    if (!state->fullscreen_lock) {
      toggleFullScreen(window);
      state->refresh_window = 1;
      state->fullscreen_lock = 1;
    }
  } else {
    state->fullscreen_lock = 0;
  }

  // Check for debug toggle
  key_state = glfwGetKey(window, KEY_DEBUG);
  if (key_state == GLFW_PRESS) {
    if (!state->debug_lock) {
      state->mode = (state->mode) ? 0 : 1;
      state->debug_lock = 1;
    }
  } else {
    state->debug_lock = 0;
  }

  // Check for step
  key_state = glfwGetKey(window, KEY_STEP);
  if (key_state == GLFW_PRESS) {
    if (!state->step_lock) {
      state->step = 1;
      state->step_lock = 1;
    }
  } else {
    state->step_lock = 0;
  }

  // Update keys
  int keys[] = {KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F};
  for (uint8_t i = 0; i < KEYS_SIZE; i++) {
    chip->keys[i] = glfwGetKey(window, keys[i]);
  }
}


static void update_window_viewport(GLFWwindow* window, int* width, int* height, State* state) {
  assert(window);
  assert(state);
  int w, h;
  glfwGetFramebufferSize(window, &w, &h);
  if (*width != w || *height != h) {
    *width = w;
    *height = h;
    glViewport(0, 0, w, h);
    state->refresh_window = 1;
  }
}

static void update_color_buffer(Chip8* chip, unsigned int vbo_color, unsigned short* color_buffer, int count) {
  assert(chip);
  assert(color_buffer);

  unsigned short* buf_ptr = color_buffer;
  uint64_t mask = 0;

  for (int y = 0; y < DISPLAY_HEGIHT; y++) {
    for (int x = DISPLAY_WIDTH - 1; x >= 0; x--) {
      mask = (uint64_t)0x1;
      mask = mask << x;
      if (chip->pixels[y] & mask) {
        buf_ptr[0] = 1;
        buf_ptr[1] = 1;
        buf_ptr[2] = 1;
        buf_ptr[3] = 1;
      } else {
        buf_ptr[0] = 0;
        buf_ptr[1] = 0;
        buf_ptr[2] = 0;
        buf_ptr[3] = 0;
      }

      buf_ptr += 4;
    }
  }

  glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
  glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(unsigned short), color_buffer);
}



static void print_debug(Chip8* chip) {
  assert(chip);

  // printf("\033[2J"); // clear screen
  printf("\n");

  uint16_t opcode;
  opcode = chip->memory[chip->PC]; // Upper byte
  opcode = opcode << 8;
  opcode = opcode | chip->memory[chip->PC + 1]; // Lower byte

  printf("PC: 0x%04X | SP: 0x%02X | I: 0x%04X | Opcode: 0x%04X\n", chip->PC, chip->SP, chip->I, opcode);
  printf("Registers: ");
  for (int i = 0; i < 16; i++) {
    printf("V[%X]: 0x%02X ", i, chip->registers[i]);
  }
  printf("\nStack: ");
  for (int i = 0; i < chip->SP; i++) {
    printf("0x%04X ", chip->stack[i]);
  }
  printf("\nDelay Timer: %d | Sound Timer: %d\n", chip->DT, chip->ST);

  printf("\n\n\n");
}
