#ifndef MANDELBROT_GLFW
#define MANDELBROT_GLFW

#include "Common.h"
#include "Mandelbrot_common.h"
#include <GLFW/glfw3.h>

int Mandelbrot_glfw_enter(GLFWwindow *restrict *restrict win_ptr, struct Mandelbrot_context *restrict context);

int Mandelbrot_glfw_finish(GLFWwindow *win);

int Mandelbrot_glfw_refresh(GLFWwindow *restrict win);

void Mandelbrot_glfw_leave(GLFWwindow *restrict win);

#endif