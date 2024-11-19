#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

#include "chip8.h"
#define WINDOW_TITLE "Chip8"
#define NORMAL_MODE 1
#define DEBUG_MODE 0
#define KEY_NORMAL 76
#define KEY_DEBUG 75
#define KEY_STEP 46

void update_keyboard_input(GLFWwindow *window, struct Chip8 *chip8) {
  int i;
  int state;
  int keys[] = {KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8,
                KEY_9, KEY_0, KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F};
  uint8_t keycodes[] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8,
                        0x9, 0x0, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf};

  for (i = 0; i < 16; i++) {
    state = glfwGetKey(window, keys[i]);
    printf("update keyboard: key: %02X keycode: %02X state: %02X\n", keys[i],
           keycodes[i], state);
    chip8_key_state(chip8, keycodes[i], state);
  }
}

GLubyte rasters[24] = {0xc0, 0x00, 0xc0, 0x00, 0xc0, 0x00, 0xc0, 0x00,
                       0xc0, 0x00, 0xff, 0x00, 0xff, 0x00, 0xc0, 0x00,
                       0xc0, 0x00, 0xc0, 0x00, 0xff, 0xc0, 0xff, 0xc0};

void display() {}

int main(int argc, char *argv[]) {
  // catch wrong args given
  if (argc > 2) {
    printf("Too many args.\n");
    return 0;
  } else if (argc < 2) {
    printf("One arg expected.\n");
    return 0;
  }

  int mode = NORMAL_MODE;
  int last_mode = NORMAL_MODE;
  int step = 0;  // used in debug mode
  struct Chip8 chip8;
  chip8_init(&chip8);
  chip8_file_into_memory(&chip8, argv[1]);  // load program into mem

  // init GLFW
  if (!glfwInit()) {
    fprintf(stderr, "Failed to init GLFW\n");
    return -1;
  }

  // open a window
  GLFWwindow *window;
  window = glfwCreateWindow(1024, 768, WINDOW_TITLE, NULL, NULL);
  if (!window) {
    fprintf(stderr, "Failed to open GLFW window.\n");
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);  // Initialize GLEW
  glewExperimental = 1;
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    return -1;
  }

  // Ensure capture the escape key being pressed
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // todo idk what this does
  do {
    // check for mode switch
    if (glfwGetKey(window, KEY_DEBUG) == GLFW_PRESS &&
        last_mode != DEBUG_MODE) {
      last_mode = mode;
      mode = DEBUG_MODE;
      printf("========================debug mode========================\n");
    } else if (glfwGetKey(window, KEY_NORMAL) == GLFW_PRESS &&
               last_mode != NORMAL_MODE) {
      last_mode = mode;
      mode = NORMAL_MODE;
      printf("========================normal mode========================\n");
    }
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // Set background color
    glClear(GL_COLOR_BUFFER_BIT);          // Clear the screen

    if (mode == DEBUG_MODE) {
      // wait for key press to do a step
      if (glfwGetKey(window, KEY_STEP) == GLFW_PRESS && step) {
        step = 0;
        // todo clock tick
        update_keyboard_input(window, &chip8);
        // todo sound timer beep
        chip8_execute(&chip8);
        chip8_std_display(&chip8);
        // todo draw to window
      } else if (glfwGetKey(window, KEY_STEP) == GLFW_RELEASE && !step) {
        step = 1;
      }
    } else {
      // normal mode just keep going though the instructions
      // todo clock tick
      update_keyboard_input(window, &chip8);
      // todo sound timer beep
      chip8_execute(&chip8);
      chip8_std_display(&chip8);
      // todo draw to window

      glColor3f(1.0, 1.0, 1.0); // set color to white
      glRasterPos2i(0, 0); // position to draw
      glBitmap(10, 12, 0.0, 0.0, 11.0, 0.0, rasters); // draw bitmap
      glFlush(); // flush
    }

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
      printf("OpenGL error: %d\n", error);
    }

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
  }  // Check if the ESC key was pressed or the window was closed
  while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
         glfwWindowShouldClose(window) == 0);
  return 0;
}