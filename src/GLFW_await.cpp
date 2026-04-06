#include "GLFW_await.h"

static byte_t callback_done = 0;
static void wait_resize_callback(GLFWwindow *win, int w, int h) {
	if (callback_done) { return; }

	callback_done = 1;
}

void await_glfwInit(GLFWwindow *window) {
    glfwSetFramebufferSizeCallback(window, wait_resize_callback);
}

void await_glfwMaximizeWindow(GLFWwindow *window) {
	callback_done = 0;
	glfwMaximizeWindow(window);
	if (!callback_done) { glfwWaitEvents(); }
}

void await_glfwRestoreWindow(GLFWwindow *window) {
	callback_done = 0;
	glfwRestoreWindow(window);
	if (!callback_done) { glfwWaitEvents(); }
}

void await_glfwSetWindowSize(GLFWwindow *window, int width, int height) {
	callback_done = 0;
	glfwSetWindowSize(window, width, height);
	if (!callback_done) { glfwWaitEvents(); }
}

void await_glfwSetWindowTitle(GLFWwindow *window, char const *title) {
	callback_done = 0;
	glfwSetWindowTitle(window, title);
	if (!callback_done) { glfwWaitEvents(); }
}