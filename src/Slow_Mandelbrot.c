#include "Slow_Mandelbrot.h"
#include <GLFW/glfw3.h>

#define GL_CALL(gl_func, ...)											\
do {																	\
	gl_func(__VA_ARGS__);												\
	GLenum __cur_err_val = glGetError();								\
	if (__cur_err_val != GL_NO_ERROR) {									\
		ON_DEBUG(														\
		fprintf(stderr, "Error found: " __FILE__ ":%d: %s: Code %u\n",	\
						__LINE__, __func__, __cur_err_val);				\
		perror(#gl_func " failed");										\
		)																\
		CLEAR_RESOURCES();												\
		return (int)__cur_err_val;										\
	}																	\
} while (false)


#define GLFW_FAILED_TO_INIT	0x100
static void error_callback(int err, char const *desc) {
	fprintf(stderr, "GLFW Error %d: %s\n", err, desc);
}

struct Mandelbrot_context {
	size_t size;
	GLfloat	(*pixels)[3],
			scale,
			x_off,
			y_off;
	GLsizei	w,
			h;
};

struct Complex {
	GLfloat	x,
			y;
};

static struct Complex add_complex(struct Complex a, struct Complex b) {
	return (struct Complex){a.x + b.x,
							a.y + b.y};
}

static struct Complex mlt_complex(struct Complex a, struct Complex b) {
	return (struct Complex){a.x * b.x - a.y * b.y,
							a.x * b.y + a.y * b.x};
}

static GLfloat abs2(struct Complex z) {
	return z.x * z.x + z.y * z.y;
}

#define MANDELBROT_ITER		((size_t) 32)
#define MANDELBROT_BORDER2	((GLfloat)4)
static void update_context(struct Mandelbrot_context *context_ptr) {
	assert(context_ptr);
	
	for (GLsizei y_it = 0; y_it < context_ptr->h; y_it++) {
		for (GLsizei x_it = 0; x_it < context_ptr->w; x_it++) {
			struct Complex	z0 = (struct Complex){	(GLfloat)(x_it - context_ptr->w / 2) * context_ptr->scale + context_ptr->x_off,
													(GLfloat)(y_it - context_ptr->h / 2) * context_ptr->scale + context_ptr->y_off},
							z = z0;

			size_t iter = 0;
			for (; iter < MANDELBROT_ITER; iter++) {
				if (abs2(z) > MANDELBROT_BORDER2) { break; }
				z = add_complex(mlt_complex(z, z), z0);
			}

			size_t cur_ind = (size_t)(y_it * context_ptr->w + x_it);
			GLfloat	t1 = (GLfloat)iter / MANDELBROT_ITER,
					t0 = 1 - t1;
			context_ptr->pixels[cur_ind][0] = 1				* t0 * t0 * t0;
			context_ptr->pixels[cur_ind][1] = (GLfloat)6.75	* t1 * t0 * t0;
			context_ptr->pixels[cur_ind][2] = (GLfloat)6.75	* t1 * t1 * t0;
		}
	}
}

#define DEFAULT_SCALE	((GLfloat)0.003)
#define SCALE_MLT		((GLfloat)1.1)
#define PIXEL_STEP		100
static void keyboard_callback(GLFWwindow *win, int key, [[maybe_unused]] int scancode, int action, int mods) {
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

static void buff_resize_callback(GLFWwindow *win, int w, int h) {
	assert(win);

	struct Mandelbrot_context *context_ptr = glfwGetWindowUserPointer(win);
	assert(context_ptr);

	context_ptr->w = w;
	context_ptr->h = h;
	glViewport(0, 0, context_ptr->w, context_ptr->h);
	size_t new_size = (size_t)(context_ptr->w * context_ptr->h) * sizeof(*context_ptr->pixels);
	if (new_size > context_ptr->size) {
		context_ptr->size = max(context_ptr->size * 2, new_size);
		void *new_pixels = realloc(context_ptr->pixels, context_ptr->size);
		if (!new_pixels) { fprintf(stderr, "I can't fix that, because glfwTerminate is not reenterable\n"); PRINT_LINE(); return; }
		context_ptr->pixels = new_pixels;
	}
}

#define FINAL_CODE

static int update_frame(GLFWwindow *win) {
	assert(win);

	struct Mandelbrot_context *context_ptr = glfwGetWindowUserPointer(win);
	update_context(context_ptr); // TODO - Disimprovement
	GL_CALL(glDrawPixels, context_ptr->w, context_ptr->h, GL_BGR, GL_FLOAT, context_ptr->pixels);
	glfwSwapBuffers(win);

	int glfw_error = glfwGetError(0);
	CLEAR_RESOURCES();
	return glfw_error == GLFW_NO_ERROR ? 0 : glfw_error;
}

#undef FINAL_CODE

#define DEFAULT_WIN_W	1024
#define DEFAULT_WIN_H	768

int run_Slow_Mandelbrot() {
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
	context.size = (size_t)(context.w * context.h) * sizeof(*context.pixels);
	context.pixels = malloc(context.size);
	if (!context.pixels) { CLEAR_RESOURCES(); return errno; }
	#undef FINAL_CODE
	#define FINAL_CODE		\
	free(context.pixels);	\
	glfwDestroyWindow(win);	\
	glfwTerminate();
	context.scale = DEFAULT_SCALE;
	context.x_off = 0;
	context.y_off = 0;
	glfwSetWindowUserPointer(win, &context);
	glfwSetKeyCallback(win, keyboard_callback);
	glfwSetFramebufferSizeCallback(win, buff_resize_callback);

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
