#include "graphics.h"
#include "chip8.h"

#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <assert.h>

typedef struct {
  float x, y;
} Vec2;

typedef struct {
  Vec2 bl, br, tr, tl;
} Square;


GLFWwindow* init_window(int width, int height, const char* title) {
  GLFWwindow* window;

  if (!glfwInit()) {
    printf("glfwInit failed\n");
    return NULL;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  window = glfwCreateWindow(width, height, title, NULL, NULL);
  if (!window) {
    glfwTerminate();
    return NULL;
  }
  glfwSetWindowAspectRatio(window, 2, 1);

  glfwMakeContextCurrent(window);

  if (glewInit() != GLEW_OK) {
    printf("glewInit failed\n");
    return NULL;
  }

  return window;
}

static Vec2 translate_to_display(const int x, const int y) {
  float new_x = ((2.0f * x) / WINDOW_WIDTH) - 1;
  float new_y = ((-2.0f * y) / WINDOW_HEIGHT) + 1;
  return (Vec2) {new_x, new_y};
}

static void populate_pos_buffers(unsigned int* vbo_pos, unsigned int* ibo) {
  int count = DISPLAY_WIDTH * DISPLAY_HEGIHT;
  unsigned short base_index[] = {0, 1, 2, 2, 3, 0};
  unsigned short index_data[count * INDEXES_PER_SQUARE];
  unsigned short* index_ptr = index_data;
  Square vertex_data[count];

  int counter = 0;
  int x = PIXEL_GAP;
  int y = PIXEL_GAP;
  for (int i = 0; i < DISPLAY_HEGIHT; i++) {
    for (int j = 0; j < DISPLAY_WIDTH; j++) {
      Square s = {
        .tl = translate_to_display(x, y),
        .tr = translate_to_display(x + PIXEL_SIZE, y),
        .bl = translate_to_display(x, y + PIXEL_SIZE),
        .br = translate_to_display(x + PIXEL_SIZE, y + PIXEL_SIZE)
      };
      vertex_data[counter] = s;

      int offset = 4 * counter;
      for (int k = 0; k < INDEXES_PER_SQUARE; k++) {
        index_ptr[k] = base_index[k] + offset;
      }
      index_ptr += INDEXES_PER_SQUARE;

      x = x + PIXEL_SIZE + PIXEL_GAP;
      counter++;
    }
    y = y + PIXEL_SIZE + PIXEL_GAP;
    x = PIXEL_GAP;
  }

  glBindBuffer(GL_ARRAY_BUFFER, *vbo_pos);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_data), index_data, GL_STATIC_DRAW);
}

int setup_buffers(unsigned int* vbo_pos, unsigned int* vbo_color, unsigned int* ibo) {
  // Vertex Array Object
  unsigned int vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // Vertex Buffer Object Positions
  glGenBuffers(1, vbo_pos);
  glBindBuffer(GL_ARRAY_BUFFER, *vbo_pos);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
  // Index Buffer Object
  glGenBuffers(1, ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ibo);
  // Vertex Buffer Object Colors
  glGenBuffers(1, vbo_color);
  glBindBuffer(GL_ARRAY_BUFFER, *vbo_color);
  glBufferData(GL_ARRAY_BUFFER, 4 * DISPLAY_HEGIHT * DISPLAY_WIDTH * sizeof(unsigned short), NULL, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 1, GL_UNSIGNED_SHORT, GL_FALSE, 0, 0);

  populate_pos_buffers(vbo_pos, ibo);

  return DISPLAY_WIDTH * DISPLAY_HEGIHT * INDEXES_PER_SQUARE;
}


void toggleFullScreen(GLFWwindow* window) {
  GLFWmonitor* monitor = glfwGetWindowMonitor(window);
  if (monitor) {
    // make windowed
    glfwSetWindowMonitor(window, NULL, 50, 50, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
  } else {
    // make full-screen
    GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
    if (primary_monitor) {
      const GLFWvidmode* mode = glfwGetVideoMode(primary_monitor);
      glfwSetWindowMonitor(window, primary_monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    }
  }
}







static void compile_shader(unsigned int shader, const char* source) {
  assert(source);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);

  int compile_status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
  if (compile_status != GL_TRUE) {
    int length = 0;
    char message[1024];
    glGetShaderInfoLog(shader, 1024, &length, message);
    printf("Shader error: %s\n", message);
  }
  assert(compile_status);
}

int install_shaders() {
  const char vertex_shader_source[] =
    "#version 330 core\n"
    "\n"
    "layout(location = 0) in vec4 position;\n"
    "layout(location = 1) in uint color;\n"
    "\n"
    "out vec3 vColor;\n"
    "\n"
    "void main() {\n"
    "  gl_Position = position;\n"
    "  if (bool(color)) {\n"
    "    vColor = vec3(1.0, 1.0, 1.0);\n"
    "  } else {\n"
    "    vColor = vec3(0.0, 0.0, 0.0);\n"
    "  }\n"
    "}";
  const char fragment_shader_source[] =
    "#version 330 core\n"
    "\n"
    "layout(location = 0) out vec4 color;\n"
    "\n"
    "in vec3 vColor;\n"
    "\n"
    "void main() {\n"
    "  color = vec4(vColor, 1.0);\n"
    "}";

  unsigned int vshader, fshader;
  vshader = glCreateShader(GL_VERTEX_SHADER);
  fshader = glCreateShader(GL_FRAGMENT_SHADER);

  compile_shader(vshader, vertex_shader_source);
  compile_shader(fshader, fragment_shader_source);

  unsigned int program = glCreateProgram();

  glAttachShader(program, vshader);
  glAttachShader(program, fshader);
  glLinkProgram(program);
  glDeleteShader(vshader);
  glDeleteShader(fshader);

  int link_status;
  glGetProgramiv(program, GL_LINK_STATUS, &link_status);
  if (link_status != GL_TRUE) {
    int length = 0;
    char message[1024];
    glGetProgramInfoLog(program, 1024, &length, message);
    printf("Linker error: %s\n", message);
    return -1;
  }

  glUseProgram(program);

  return 0;
}

