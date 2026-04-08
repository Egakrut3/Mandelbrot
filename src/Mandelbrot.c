#include "Mandelbrot.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>

// TODO - Make error handling
// TODO - Possible use BGRA

#define GLFW_FAILED_TO_INIT				1
#define GLFW_FAILED_TO_CREATE_WINDOW	2
static void error_callback(int err, char const *desc) {
	fprintf(stderr, "GLFW Error %d: %s\n", err, desc);
}

struct Pixel_buff {
	GLubyte *pixels;
	GLsizei	w,
			h;
};

static void fill_buff(struct Pixel_buff *buff_ptr) {
	for (GLsizei y = 0; y < buff_ptr->h; y++) {
		for (GLsizei x = 0; x < buff_ptr->w; x++) {
			GLubyte *cur_pixel = buff_ptr->pixels + (y * buff_ptr->w + x) * 4;
			cur_pixel[0] = x		& 0xFF;
			cur_pixel[1] = y		& 0xFF;
			cur_pixel[2] = (x + y)	& 0xFF;

			cur_pixel[3] = 0xFF;
		}
	}
}

static void update_win(GLFWwindow *win) {
	struct Pixel_buff *buff_ptr = glfwGetWindowUserPointer(win);
	fill_buff(buff_ptr);

	glViewport(0, 0, buff_ptr->w, buff_ptr->h);
	glDrawPixels(buff_ptr->w, buff_ptr->h, GL_RGBA, GL_UNSIGNED_BYTE, buff_ptr->pixels);
	glfwSwapBuffers(win);
	glDrawPixels(buff_ptr->w, buff_ptr->h, GL_RGBA, GL_UNSIGNED_BYTE, buff_ptr->pixels);
}

static void buff_resize_callback(GLFWwindow *win, GLsizei w, GLsizei h) {
	struct Pixel_buff *buff_ptr = glfwGetWindowUserPointer(win);
	buff_ptr->w = w;
	buff_ptr->h = h;
	buff_ptr->pixels = realloc(buff_ptr->pixels, buff_ptr->w * buff_ptr->h * 4 * sizeof(*buff_ptr->pixels));

	update_win(win);
}

#define start_win_w	800
#define start_win_h	600

int run_Mandelbrot() {
	glfwSetErrorCallback(error_callback);
	glfwInit();

	GLFWwindow *win = glfwCreateWindow(start_win_w, start_win_h, "FPS: ", 0, 0);
	glfwMakeContextCurrent(win);
	glfwSwapInterval(1);

	struct Pixel_buff buff = {};
	glfwGetFramebufferSize(win, &buff.w, &buff.h);
	buff.pixels = malloc(buff.w * buff.h * 4 * sizeof(*buff.pixels));
	glfwSetWindowUserPointer(win, &buff);
	update_win(win);
	glfwSetFramebufferSizeCallback(win, buff_resize_callback);

	#define MAX_FPS_TITLE_LENGTH 16
	double	FPS = 0,
			last_FRS_rep_time	= glfwGetTime(),
			frm_beg_time		= last_FRS_rep_time;
	char FPS_title[MAX_FPS_TITLE_LENGTH] = "";
	while (!glfwWindowShouldClose(win)) {
		glfwSwapBuffers(win);
		glfwPollEvents();

		double cur_time = glfwGetTime();
		FPS = FPS * 0.9 + 1 / (cur_time - frm_beg_time) * 0.1;
		if (cur_time - last_FRS_rep_time >= 1) {
			snprintf(FPS_title, MAX_FPS_TITLE_LENGTH, "FPS: %.2f", FPS);
			glfwSetWindowTitle(win, FPS_title);
			last_FRS_rep_time = cur_time;
		}
		frm_beg_time = cur_time;
	}

	free(buff.pixels);
	glfwDestroyWindow(win);

	glfwTerminate();
	return 0;
}
