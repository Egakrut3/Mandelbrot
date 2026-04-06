#ifndef GLFW_AWAIT
#define GLFW_AWAIT

#include "Common.h"
#include <GLFW/glfw3.h>

void await_glfwInit(GLFWwindow *window);



void await_glfwMaximizeWindow(GLFWwindow *window);

void await_glfwRestoreWindow(GLFWwindow *window);

void await_glfwSetWindowSize(GLFWwindow *window, int width, int height);

void await_glfwSetWindowTitle(GLFWwindow *window, char const *title);

#endif