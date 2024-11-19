#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

#include "chip8.h"
#define WINDOW_TITLE "Chip8"


void update_keyboard_input(GLFWwindow *window, struct Chip8 *chip8) {
  int i;
  int state;
  int keys[] = {KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8,
                KEY_9, KEY_0, KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F};

  for (i = 0; i < 16; i++) {
    state = glfwGetKey(window, keys[i]);
    chip8_key_state(chip8, keys[i], state);
  }
}



int main(int argc, char *argv[]) {
  // catch wrong args given
  if (argc > 2) {
    printf("Too many args.\n");
    return 0;
  } else if (argc < 2) {
    printf("One arg expected.\n");
    return 0;
  }

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

  do {
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);  // Set background color
    glClear(GL_COLOR_BUFFER_BIT);          // Clear the screen

    // todo clock tick
    update_keyboard_input(window, &chip8);
    // todo sound timer beep
    chip8_execute(&chip8);
    chip8_std_display(&chip8);
    //todo draw to window

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