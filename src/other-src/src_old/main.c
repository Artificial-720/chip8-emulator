#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

#include "chip8.h"
#define true 1
#define WINDOW_TITLE "Chip8"

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  printf("we had a key press\n");
}

void update_keyboard_input(GLFWwindow *window, struct Chip8 *chip8) {
  int i;
  int state;
  int keys[] = {KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8,
                KEY_9, KEY_0, KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F};

  for (i = 0; i < 16; i++) {
    state = glfwGetKey(window, keys[i]);
    change_key_state(chip8, keys[i], state);
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

  struct Chip8 chip8;  // create chip
  init_chip8(&chip8);  // init chip
  uint8_t *mem_start = chip8.memory + 0x200;
  read_file_contents(argv[1], mem_start);  // load prog into memory
  // // start loop
  // // while true:
  // //   clock tick
  // //   handle keyboard input
  // //   sound timer beep?
  // //   execute next instuction
  // //   display screen
  // // for (int i = 0; i < 7; i++) {
  // while (1) {
  //   execute(&chip8);
  //   display(chip8.pixels, DISPLAY_HEIGHT);
  // }

  // init GLFW
  glewExperimental = true;
  if (!glfwInit()) {
    fprintf(stderr, "Failed to init GLFW\n");
    return -1;
  }

  // glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
  // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
  // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS
  // happy; should not be needed glfwWindowHint(GLFW_OPENGL_PROFILE,
  // GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL

  // Open a window and create its OpenGL context
  GLFWwindow *window;  // (In the accompanying source code, this variable is
                       // global for simplicity)
  window = glfwCreateWindow(1024, 768, WINDOW_TITLE, NULL, NULL);
  if (window == NULL) {
    fprintf(stderr,
            "Failed to open GLFW window. If you have an Intel GPU, they are "
            "not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);  // Initialize GLEW
  glewExperimental = true;         // Needed in core profile
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    return -1;
  }

  // Ensure we can capture the escape key being pressed below
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

  // set callback for key presses
  // glfwSetKeyCallback(window, key_callback);

  // vertices for a triangle
  float vertices[] = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f};
  // vertex buffer
  unsigned int VBO;
  glGenBuffers(1, &VBO);
  // bind newly created buffer as an array buffer
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  // copy user data into the current buffer
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  // compile a shader, shader has to be compiled at run time
  const char *vertexShaderSource =
      "#version 330 core\n"
      "layout (location = 0) in vec3 aPos;\n"
      "void main()\n"
      "{\n"
      "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
      "}\0";
  // create shader object for a vertex shader
  unsigned int vertexShader;
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  // attach source code to shader
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);
  // to check for errors
  int success;
  char infoLog[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n");
    printf("%s\n", infoLog);
  }
  // fragment shader for color
  const char *fragmentShaderSource =
      "#version 330 core\n"
      "out vec4 FragColor;\n"
      "void main()\n"
      "{\n"
      "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
      "}\0";
  unsigned int fragmentShader;
  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);
  // shader program. combine multiple shaders into one program
  unsigned int shaderProgram;  // create program obj
  shaderProgram = glCreateProgram();
  // attach shaders
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  // can check if linking failed
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    printf("ERROR::SHADER::PROGRAM::COMPILATION_FAILED\n");
    printf("%s\n", infoLog);
  }
  // use the program
  glUseProgram(shaderProgram);

  // tell opengl how to interpret vertex data
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  do {
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);  // Set background color
    glClear(GL_COLOR_BUFFER_BIT);          // Clear the screen

    // clock tick
    update_keyboard_input(window, &chip8);
    // sound timer beep thing
    execute(&chip8);
    display(chip8.pixels, DISPLAY_HEIGHT);
    glDrawArrays(GL_TRIANGLES, 0, 3);

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