#ifndef GLFW_AWAIT
#define GLFW_AWAIT

#include "Common.h"
#include <GLFW/glfw3.h>

GLFWwindow *await_glfwCreateWindow(int width, int height, char const *title, GLFWmonitor *monitor, GLFWwindow *share);

void await_glfwDestroyWindow(GLFWwindow *window);

void await_glfwMaximizeWindow(GLFWwindow *window);

void await_glfwRestoreWindow(GLFWwindow *window);

void await_glfwSetWindowSize(GLFWwindow *window, int width, int height);

void await_glfwSetWindowTitle(GLFWwindow *window, char const *title);

#endif