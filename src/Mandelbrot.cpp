#include "Mandelbrot.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <cstdio>

// TODO - Make error handling
// TODO - Possible use BGRA

#define GLFW_FAILED_TO_INIT				1
#define GLFW_FAILED_TO_CREATE_WINDOW	2

static int callback_done = 0;
static void glfw_error_callback(int err, char const *desc) {
	fprintf(stderr, "GLFW Error %d: %s\n", err, desc);
}

static void wait_resize_callback(GLFWwindow *win, GLsizei w, GLsizei h) {
	if (callback_done) { return; }

	callback_done = 1;
}

#define glfw_await_MaximizeWindow(win)			\
do {											\
	callback_done = 0;							\
	glfwMaximizeWindow(win);					\
	if (!callback_done) { glfwWaitEvents(); }	\
} while (false)

#define glfw_await_RestoreWindow(win)			\
do {											\
	callback_done = 0;							\
	glfwRestoreWindow(win);						\
	if (!callback_done) { glfwWaitEvents(); }	\
} while (false)

#define glfw_await_SetWindowSize(win, w, h)		\
do {											\
	callback_done = 0;							\
	glfwSetWindowSize(win, w, h);				\
	if (!callback_done) { glfwWaitEvents(); }	\
} while (false)

#define start_w	800
#define start_h	600

int run_Mandelbrot() {
	if (!glfwInit()) { return GLFW_FAILED_TO_INIT; }
	glfwSetErrorCallback(glfw_error_callback);

	GLFWwindow *win = glfwCreateWindow(start_w, start_h, "Mandelbrot", nullptr, nullptr);
	if (!win) { glfwTerminate(); return GLFW_FAILED_TO_CREATE_WINDOW; }
	glfwMakeContextCurrent(win);
	glfwSetFramebufferSizeCallback(win, wait_resize_callback);



	glfw_await_MaximizeWindow(win);
	GLsizei	max_win_w = 0,
			max_win_h = 0;
	glfwGetWindowSize(win, &max_win_w, &max_win_h);
	glfwSetWindowSizeLimits(win, GLFW_DONT_CARE, GLFW_DONT_CARE, max_win_w, max_win_h);

	GLsizei	max_buff_w = 0,
			max_buff_h = 0;
	glfwGetFramebufferSize(win, &max_buff_w, &max_buff_h);
	GLubyte *pixels = nullptr;
	pixels = (typeof pixels)calloc(max_buff_w * max_buff_h * 4, sizeof *pixels);
	assert(pixels);

	glfw_await_RestoreWindow(win);
	glfw_await_SetWindowSize(win, start_w, start_h);



	GLsizei	cur_win_w = 0,
			cur_win_h = 0;
	glfwGetWindowSize(win, &cur_win_w, &cur_win_h);

	GLsizei	cur_buff_w = 0,
			cur_buff_h = 0;
	glfwGetFramebufferSize(win, &cur_buff_w, &cur_buff_h);
	for (size_t y = 0; y < cur_buff_h; y++) {
		for (size_t x = 0; x < cur_buff_w; x++) {
			size_t ind = (y * cur_buff_w + x) * 4;
			pixels[ind + 0] = x & 0xFF;
			pixels[ind + 1] = y & 0xFF;
			pixels[ind + 2] = (x + y) & 0xFF;
			pixels[ind + 3] = 0xFF;
		}
	}
	glOrtho(0, cur_buff_w, 0, cur_buff_h, -1, 1);



	double	frm_beg_time	= glfwGetTime(),
			FPS				= 0;
	char FPS_title[16] = "";
	while (!glfwWindowShouldClose(win)) {
		GLsizei	new_win_w = 0,
				new_win_h = 0;
		glfwGetWindowSize(win, &new_win_w, &new_win_h);
		new_win_w = new_win_w <= max_win_w ? new_win_w : max_win_w;
		new_win_h = new_win_h <= max_win_h ? new_win_h : max_win_h;
		if (new_win_w != cur_win_w or
			new_win_h != cur_win_h) {
			glfw_await_SetWindowSize(win, new_win_w, new_win_h);



			cur_win_w = new_win_w;
			cur_win_h = new_win_h;

			glfwGetFramebufferSize(win, &cur_buff_w, &cur_buff_h);
			for (size_t y = 0; y < cur_buff_h; y++) {
				for (size_t x = 0; x < cur_buff_w; x++) {
					size_t ind = (y * cur_buff_w + x) * 4;
					pixels[ind + 0] = x & 0xFF;
					pixels[ind + 1] = y & 0xFF;
					pixels[ind + 2] = (x + y) & 0xFF;
					pixels[ind + 3] = 0xFF;
				}
			}
			glOrtho(0, cur_buff_w, 0, cur_buff_h, -1, 1);
		}
		
		glDrawPixels(cur_buff_w, cur_buff_h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		glfwSwapBuffers(win);

		double cur_time = glfwGetTime();
		FPS = FPS * 0.9 + 0.1 / (cur_time - frm_beg_time);
		frm_beg_time = cur_time;
		snprintf(FPS_title, 16, "FPS: %.2f", FPS);
		glfwSetWindowTitle(win, FPS_title);

		glfwPollEvents();
	}

	free(pixels);
	glfwDestroyWindow(win);
	glfwTerminate();
	return 0;
}
