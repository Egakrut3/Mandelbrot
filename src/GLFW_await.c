#include "GLFW_await.h"
#include <GLFW/glfw3.h>

static byte_t callback_done = 0;
static void wait_resize_callback(GLFWwindow *win, int w, int h) {
	if (callback_done) { return; }

	callback_done = 1;
}



GLFWwindow *await_glfwCreateWindow(int w, int h, char const *title, GLFWmonitor *monitor, GLFWwindow *share) {
	GLFWwindow *win = glfwCreateWindow(w, h, title, monitor, share);
	glfwSetFramebufferSizeCallback(win, wait_resize_callback);
	return win;
}

void await_glfwDestroyWindow(GLFWwindow *win) {
	glfwDestroyWindow(win);
}



void await_glfwMaximizeWindow(GLFWwindow *win) {
	callback_done = 0;
	glfwMaximizeWindow(win);
	if (!callback_done) { glfwWaitEvents(); }
}

void await_glfwRestoreWindow(GLFWwindow *win) {
	callback_done = 0;
	glfwRestoreWindow(win);
	if (!callback_done) { glfwWaitEvents(); }
}

void await_glfwSetWindowSize(GLFWwindow *win, int w, int h) {
	callback_done = 0;
	glfwSetWindowSize(win, w, h);
	if (!callback_done) { glfwWaitEvents(); }
}

void await_glfwSetWindowTitle(GLFWwindow *win, char const *title) {
	callback_done = 0;
	glfwSetWindowTitle(win, title);
	if (!callback_done) { glfwWaitEvents(); }
}