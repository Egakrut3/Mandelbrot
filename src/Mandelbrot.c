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
struct Pixel_buffer {
	GLubyte *pixels;
	GLsizei	w,
			h;
};

static void buff_resize_callback(GLFWwindow *win, GLsizei w, GLsizei h) {
	struct Pixel_buffer *buff_ptr = glfwGetWindowUserPointer(win);
	buff_ptr->w = w;
	buff_ptr->h = h;
	buff_ptr->pixels = realloc(buff_ptr->pixels, buff_ptr->w * buff_ptr->h * 4 * sizeof(*buff_ptr->pixels));
	for (GLsizei y = 0; y < buff_ptr->h; y++) {
		for (GLsizei x = 0; x < buff_ptr->w; x++) {
			GLubyte *cur_pixel = buff_ptr->pixels + (y * buff_ptr->w + x) * 4;
			cur_pixel[0] = x		& 0xFF;
			cur_pixel[1] = y		& 0xFF;
			cur_pixel[2] = (x + y)	& 0xFF;

			cur_pixel[3] = 0xFF;
		}
	}
	glViewport(0, 0, buff_ptr->w, buff_ptr->h);
}

#define start_win_w	800
#define start_win_h	600

int run_Mandelbrot() {
	glfwSetErrorCallback(error_callback);
	glfwInit();



	glfwWindowHint(GLFW_STEREO,				GLFW_FALSE);
	glfwWindowHint(GLFW_DOUBLEBUFFER,			GLFW_TRUE);
	glfwWindowHint(GLFW_CLIENT_API,			GLFW_OPENGL_API);

	//glfwWindowHint(GLFW_RESIZABLE,			GLFW_FALSE);
	// glfwWindowHint(GLFW_SCALE_FRAMEBUFFER,	GLFW_FALSE);



	GLFWwindow *win = glfwCreateWindow(start_win_w, start_win_h, "FPS: ", 0, 0);
	glfwMakeContextCurrent(win);
	glfwSwapInterval(1);



	struct Pixel_buffer buff = {};
	glfwGetFramebufferSize(win, &buff.w, &buff.h);
	buff.pixels = malloc(buff.w * buff.h * 4 * sizeof(*buff.pixels));
	for (GLsizei y = 0; y < buff.h; y++) {
		for (GLsizei x = 0; x < buff.w; x++) {
			GLubyte *cur_pixel = buff.pixels + (y * buff.w + x) * 4;
			cur_pixel[0] = x		& 0xFF;
			cur_pixel[1] = y		& 0xFF;
			cur_pixel[2] = (x + y)	& 0xFF;

			cur_pixel[3] = 0xFF;
		}
	}
	glViewport(0, 0, buff.w, buff.h);
	glfwSetWindowUserPointer(win, &buff);
	glfwSetFramebufferSizeCallback(win, buff_resize_callback);



	#define MAX_FPS_TITLE_LENGTH 16
	double	FPS = 0,
			last_FRS_rep_time	= glfwGetTime(),
			frm_beg_time		= last_FRS_rep_time;
	char FPS_title[MAX_FPS_TITLE_LENGTH] = "";
	while (!glfwWindowShouldClose(win)) {
		glDrawPixels(buff.w, buff.h, GL_RGBA, GL_UNSIGNED_BYTE, buff.pixels);
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
