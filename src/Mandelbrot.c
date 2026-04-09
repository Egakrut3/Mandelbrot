#include "Mandelbrot.h"
#include <GLFW/glfw3.h>

// TODO - Make error handling
// TODO - Possible use BGRA

#define GL_CALL(gl_func, ...)											\
do {																	\
	gl_func(__VA_ARGS__);												\
	GLenum __cur_err_val = glGetError();								\
	if (__cur_err_val != GL_NO_ERROR) {									\
		ON_DEBUG(														\
		fprintf(stderr, "Error found: " __FILE__ ":%d: %s: Code %d\n",	\
						__LINE__, __func__, __cur_err_val);				\
		perror(#gl_func " failed");										\
		)																\
		CLEAR_RESOURCES();												\
		return __cur_err_val;											\
	}																	\
} while (false);


#define GLFW_FAILED_TO_INIT	0x100
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

#define MANDELBROT_ITER		((size_t) 32)
#define MANDELBROT_BORDER2	2 * 2

void load_RGBA_FLOAT_color(struct Complex z0, GLfloat *buff) {
	assert(buff);

	struct Complex z = z0;
	size_t i = 0;
	for (; i < MANDELBROT_ITER; z = sum_Complex(mlt_Complex(z, z), z0), i++) {
		if (abs2(z) > MANDELBROT_BORDER2) {
			break;
		}
	}

	GLfloat	t1 = (GLfloat) i / MANDELBROT_ITER,
			t0 = 1 - t1;
	buff[2] = (GLfloat)1.	* t0 * t0 * t0;
	buff[0] = (GLfloat)6.75	* t1 * t0 * t0;
	buff[1] = (GLfloat)6.75	* t1 * t1 * t0;
	return;
}

struct Mandelbrot_context {
	GLfloat	*pixels,
			scale,
			x_off,
			y_off;
	size_t size;
	GLsizei	w,
			h;
};

static void update_context(struct Mandelbrot_context *context_ptr) {
	assert(context_ptr);

	for (GLsizei y = 0; y < context_ptr->h; y++) {
		for (GLsizei x = 0; x < context_ptr->w; x++) {
			GLfloat *cur_pixel = context_ptr->pixels + (y * context_ptr->w + x) * 3;
			struct Complex z0 = (struct Complex){	(x - context_ptr->w / 2) * context_ptr->scale + context_ptr->x_off,
													(y - context_ptr->h / 2) * context_ptr->scale + context_ptr->y_off};
			load_RGBA_FLOAT_color(z0, cur_pixel);
		}
	}
}

static void buff_resize_callback(GLFWwindow *win, GLsizei w, GLsizei h) {
	assert(win);

	struct Mandelbrot_context *context_ptr = glfwGetWindowUserPointer(win);
	assert(context_ptr);

	context_ptr->w = w;
	context_ptr->h = h;
	glViewport(0, 0, context_ptr->w, context_ptr->h);
	size_t new_size = context_ptr->w * context_ptr->h * 3 * sizeof(*context_ptr->pixels);
	if (new_size > context_ptr->size) {
		context_ptr->size = max(new_size, context_ptr->size * 2);
		void *new_pixels = realloc(context_ptr->pixels, context_ptr->size);
		if (!new_pixels) { fprintf(stderr, "I can't fix that\n"); PRINT_LINE(); return; }
		context_ptr->pixels = new_pixels;
	}
}

#define DEFAULT_SCALE	((GLfloat)0.003)
#define SCALE_MLT		((GLfloat)1.1)
#define PIXEL_STEP		100
static void keyboard_callback(GLFWwindow *win, int key, int scancode, int action, int mods) {
	assert(win);

	struct Mandelbrot_context *context_ptr = glfwGetWindowUserPointer(win);
	assert(context_ptr);

	if (action == GLFW_PRESS) {
		if (mods & GLFW_MOD_SHIFT) {
			if (key == GLFW_KEY_EQUAL)	{ context_ptr->scale /= SCALE_MLT; }
			if (key == GLFW_KEY_MINUS)	{ context_ptr->scale *= SCALE_MLT; }
		}

		if (key == GLFW_KEY_RIGHT)	{ context_ptr->x_off += context_ptr->scale * PIXEL_STEP; }
		if (key == GLFW_KEY_LEFT)	{ context_ptr->x_off -= context_ptr->scale * PIXEL_STEP; }
		if (key == GLFW_KEY_UP)		{ context_ptr->y_off += context_ptr->scale * PIXEL_STEP; }
		if (key == GLFW_KEY_DOWN)	{ context_ptr->y_off -= context_ptr->scale * PIXEL_STEP; }
	}
}

#define FINAL_CODE

static int update_frame(GLFWwindow *win) {
	assert(win);

	struct Mandelbrot_context *context_ptr = glfwGetWindowUserPointer(win);
	update_context(context_ptr); // TODO -
	GL_CALL(glDrawPixels, context_ptr->w, context_ptr->h, GL_RGB, GL_FLOAT, context_ptr->pixels);
	glfwSwapBuffers(win);

	int glfw_error = glfwGetError(0);
	CLEAR_RESOURCES();
	return glfw_error == GLFW_NO_ERROR ? 0 : glfw_error;
}

#undef FINAL_CODE

#define DEFAULT_WIN_W	800
#define DEFAULT_WIN_H	600

int run_Mandelbrot() {
	#define FINAL_CODE

	glfwSetErrorCallback(error_callback);
	if (glfwInit() == GLFW_FALSE) { CLEAR_RESOURCES(); return glfwGetError(0); }
	#undef FINAL_CODE
	#define FINAL_CODE	\
	glfwTerminate();

	GLFWwindow *win = glfwCreateWindow(DEFAULT_WIN_W, DEFAULT_WIN_H, "FPS: ", 0, 0);
	if (!win) { CLEAR_RESOURCES(); return glfwGetError(0); }
	#undef FINAL_CODE
	#define FINAL_CODE		\
	glfwDestroyWindow(win);	\
	glfwTerminate();

	glfwMakeContextCurrent(win);
	glfwSwapInterval(1);

	struct Mandelbrot_context context = {};
	glfwGetFramebufferSize(win, &context.w, &context.h);
	glViewport(0, 0, context.w, context.h);
	context.size = context.w * context.h * 3 * sizeof(*context.pixels);
	context.pixels = malloc(context.size);
	if (!context.pixels) { CLEAR_RESOURCES(); return errno; }
	#undef FINAL_CODE
	#define FINAL_CODE		\
	free(context.pixels);	\
	glfwDestroyWindow(win);	\
	glfwTerminate();
	context.scale = DEFAULT_SCALE;
	glfwSetWindowUserPointer(win, &context);
	glfwSetFramebufferSizeCallback(win, buff_resize_callback);
	glfwSetKeyCallback(win, keyboard_callback);

	#define MAX_FPS_TITLE_LENGTH	((size_t)16)
	#define SMOOTH_COEF				0.9
	double	FPS = 0,
			last_FPS_rep_time	= glfwGetTime(),
			frm_beg_time		= last_FPS_rep_time;
	char FPS_title[MAX_FPS_TITLE_LENGTH] = "";
	while (!glfwWindowShouldClose(win)) {
		CHECK_FUNC(update_frame, win);
		glfwWaitEvents();

		double cur_time = glfwGetTime();
		FPS = FPS * SMOOTH_COEF + 1 / (cur_time - frm_beg_time) * (1 - SMOOTH_COEF);
		if (cur_time - last_FPS_rep_time >= 1) {
			snprintf(FPS_title, MAX_FPS_TITLE_LENGTH, "FPS: %.2f", FPS);
			glfwSetWindowTitle(win, FPS_title);
			//fprintf(stderr, "FPS: %.2f\n", FPS);
			last_FPS_rep_time = cur_time;
		}
		frm_beg_time = cur_time;
	}

	int glfw_error = glfwGetError(0);
	CLEAR_RESOURCES();
	return glfw_error == GLFW_NO_ERROR ? 0 : glfw_error;

	#undef FINAL_CODE
}
