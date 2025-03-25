#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 512
#define PIXEL_SIZE 14
#define PIXEL_GAP 2
#define INDEXES_PER_SQUARE 6

GLFWwindow* init_window(int width, int height, const char* title);
int install_shaders();
int setup_buffers(unsigned int* vbo_pos, unsigned int* vbo_color, unsigned int* ibo);

void toggleFullScreen(GLFWwindow* window);

#endif
