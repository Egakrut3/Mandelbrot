#include "Mandelbrot.h"
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

void load_RGBA_FLOAT_color(struct Complex z0, GLfloat *buff) {
	struct Complex z = z0;
	size_t i = 0;
	for (; i < MANDELBROT_ITER; z = sum_Complex(mlt_Complex(z, z), z0), i++) {
		if (abs2(z) > MANDELBROT_BORDER2) {
			break;
		}
	}

	GLfloat	t1 = (float) i / MANDELBROT_ITER,
			t0 = 1 - t1;
	buff[2] = 1		* t0 * t0 * t0;
	buff[0] = 6.75	* t1 * t0 * t0;
	buff[1] = 6.75	* t1 * t1 * t0;
	return;
}

struct Pixel_buff {
	GLfloat *pixels;
	size_t size;
	GLsizei	w,
			h;
};

struct Mandelbrot_context {
	struct Pixel_buff buff;
};

#define MANDELBROT_SCALE 0.003f

static void fill_buff(struct Pixel_buff *buff_ptr) {
	for (GLsizei y = 0; y < buff_ptr->h; y++) {
		for (GLsizei x = 0; x < buff_ptr->w; x++) {
			GLfloat *cur_pixel = buff_ptr->pixels + (y * buff_ptr->w + x) * 3;
			struct Complex z0 = (struct Complex){	(x - buff_ptr->w / 2) * MANDELBROT_SCALE,
													(y - buff_ptr->h / 2) * MANDELBROT_SCALE};
			load_RGBA_FLOAT_color(z0, cur_pixel);
		}
	}
}

static void buff_resize_callback(GLFWwindow *win, GLsizei w, GLsizei h) {
	struct Mandelbrot_context *context_ptr = glfwGetWindowUserPointer(win);
	context_ptr->buff.w = w;
	context_ptr->buff.h = h;
	size_t new_size = context_ptr->buff.w * context_ptr->buff.h * 3 * sizeof(*context_ptr->buff.pixels);
	if (new_size > context_ptr->buff.size) {
		context_ptr->buff.size *= 2;
		context_ptr->buff.pixels = realloc(context_ptr->buff.pixels, context_ptr->buff.size);
	}

	fill_buff(&context_ptr->buff);

	glViewport(0, 0, context_ptr->buff.w, context_ptr->buff.h);
}

#define start_win_w	800
#define start_win_h	600

int run_Mandelbrot() {
	glfwSetErrorCallback(error_callback);
	glfwInit();

	GLFWwindow *win = glfwCreateWindow(start_win_w, start_win_h, "FPS: ", 0, 0);
	glfwMakeContextCurrent(win);
	glfwSwapInterval(1);

	struct Mandelbrot_context context = {};
	glfwGetFramebufferSize(win, &context.buff.w, &context.buff.h);
	context.buff.size = context.buff.w * context.buff.h * 3 * sizeof(*context.buff.pixels);
	context.buff.pixels = malloc(context.buff.size);
	glfwSetWindowUserPointer(win, &context);
	fill_buff(&context.buff);
	glViewport(0, 0, context.buff.w, context.buff.h);
	glfwSetFramebufferSizeCallback(win, buff_resize_callback);

	#define MAX_FPS_TITLE_LENGTH ((size_t)16)
	double	FPS = 0,
			last_FPS_rep_time	= glfwGetTime(),
			frm_beg_time		= last_FPS_rep_time;
	char FPS_title[MAX_FPS_TITLE_LENGTH] = "";
	while (!glfwWindowShouldClose(win)) {
		glDrawPixels(context.buff.w, context.buff.h, GL_RGB, GL_FLOAT, context.buff.pixels);

		glfwSwapBuffers(win);
		glfwWaitEvents();

		double cur_time = glfwGetTime();
		FPS = FPS * 0.9 + 1 / (cur_time - frm_beg_time) * 0.1;
		if (cur_time - last_FPS_rep_time >= 1) {
			snprintf(FPS_title, MAX_FPS_TITLE_LENGTH, "FPS: %.2f", FPS);
			glfwSetWindowTitle(win, FPS_title);
			last_FPS_rep_time = cur_time;
		}
		frm_beg_time = cur_time;
	}

	free(context.buff.pixels);
	glfwDestroyWindow(win);

	glfwTerminate();
	return 0;
}
