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

struct Complex {
	GLfloat	x,
			y;
};

struct Complex sum_Complex(struct Complex a, struct Complex b) {
	return (struct Complex){a.x + b.x,
							a.y + b.y};
}

struct Complex mlt_Complex(struct Complex a, struct Complex b) {
	return (struct Complex){a.x * b.x - a.y * b.y,
							a.x * b.y + a.y * b.x};
}

GLfloat abs2(struct Complex z) {
	return z.x * z.x + z.y * z.y;
}

#define MANDELBROT_ITER		((size_t) 64)
#define MANDELBROT_BORDER2	2 * 2

GLuint get_color(struct Complex z0) {
	struct Complex z = z0;
	size_t i = 0;
	for (; i < MANDELBROT_ITER; z = sum_Complex(mlt_Complex(z, z), z0), i++) {
		if (abs2(z) > MANDELBROT_BORDER2) {
			break;
		}
	}

	GLfloat	t1 = (float) i / MANDELBROT_ITER,
			t0 = 1 - t1;
	return	(GLubyte)(1			* t0 * t0 * t0 * 0xFF) << CHAR_BIT * 2 |
			(GLubyte)(27. / 4	* t1 * t0 * t0 * 0xFF) << CHAR_BIT * 0 |
			(GLubyte)(27. / 4	* t1 * t1 * t0 * 0xFF) << CHAR_BIT * 1 |
			(GLubyte)1 << CHAR_BIT * 3;
}

struct Pixel_buff {
	GLuint *pixels;
	size_t size;
	GLsizei	w,
			h;
};

#define MANDELBROT_SCALE 0.003f

static void fill_buff(struct Pixel_buff *buff_ptr) {
	for (GLsizei y = 0; y < buff_ptr->h; y++) {
		for (GLsizei x = 0; x < buff_ptr->w; x++) {
			GLuint *cur_pixel = buff_ptr->pixels + y * buff_ptr->w + x;
			*cur_pixel = get_color((struct Complex){(x - buff_ptr->w / 2) * MANDELBROT_SCALE,
														(y - buff_ptr->h / 2) * MANDELBROT_SCALE});
		}
	}
}

static void update_win(GLFWwindow *win) {
	struct Pixel_buff *buff_ptr = glfwGetWindowUserPointer(win);
	fill_buff(buff_ptr);

	glViewport(0, 0, buff_ptr->w, buff_ptr->h);
}

static void buff_resize_callback(GLFWwindow *win, GLsizei w, GLsizei h) {
	struct Pixel_buff *buff_ptr = glfwGetWindowUserPointer(win);
	buff_ptr->w = w;
	buff_ptr->h = h;
	size_t new_size = buff_ptr->w * buff_ptr->h * 4 * sizeof(*buff_ptr->pixels);
	if (new_size > buff_ptr->size) {
		buff_ptr->pixels = realloc(buff_ptr->pixels, new_size);
	}

	// update_win(win);
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
	buff.size = buff.w * buff.h * 4 * sizeof(*buff.pixels);
	buff.pixels = malloc(buff.size);
	glfwSetWindowUserPointer(win, &buff);
	update_win(win);
	glfwSetFramebufferSizeCallback(win, buff_resize_callback);

	#define MAX_FPS_TITLE_LENGTH ((size_t)16)
	double	FPS = 0,
			last_FRS_rep_time	= glfwGetTime(),
			frm_beg_time		= last_FRS_rep_time;
	char FPS_title[MAX_FPS_TITLE_LENGTH] = "";
	while (!glfwWindowShouldClose(win)) {
		update_win(win);
		glDrawPixels(buff.w, buff.h, GL_RGBA, GL_UNSIGNED_BYTE, buff.pixels);

		glfwSwapBuffers(win);
		glfwWaitEvents();

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
