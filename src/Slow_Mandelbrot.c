#include "Mandelbrot.h"
#include <GLFW/glfw3.h>
#include <immintrin.h>

// #define NO_DRAWING
// #define TESTING

#ifdef TESTING
#include <math.h>
#elifdef NO_DRAWING
#include <math.h>
#endif

#ifndef NO_DRAWING

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

#endif



#define DEFAULT_SCALE	((GLfloat)0.003)
#define SCALE_MLT		((GLfloat)1.1)
#define TEST_SCALE_CNT	((GLfloat)-0x20)

#ifndef NO_DRAWING

static void buff_resize_callback(GLFWwindow *win, int w, int h) {
	assert(win);

	struct Mandelbrot_context *context_ptr = glfwGetWindowUserPointer(win);
	assert(context_ptr);

	context_ptr->w = w;
	context_ptr->h = h;
	glViewport(0, 0, context_ptr->w, context_ptr->h);
	size_t new_buff_size = (size_t)(context_ptr->w * context_ptr->h) * sizeof(*context_ptr->pixels);

	if (new_buff_size > context_ptr->buff_size) {
		context_ptr->buff_size = max(context_ptr->buff_size * 2, new_buff_size);
		void *new_pixels = realloc(context_ptr->pixels, context_ptr->buff_size);
		if (!new_pixels) { PRINT_LINE(); return; } // I can't handle it, because glfwTerminate is not reenterable
		context_ptr->pixels = new_pixels;
	}
}

#define PIXEL_STEP ((GLfloat)160)
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

#endif

#define MANDELBROT_ITER		((size_t)0x40)
#define MANDELBROT_BORDER2	((GLfloat)4)
static void store_BGR_color(size_t iter, GLfloat dest[3]) {
	GLfloat	t1 = (GLfloat)iter / MANDELBROT_ITER,
			t0 = 1 - t1;
	dest[0] = 1				* t0 * t0 * t0;
	dest[1] = (GLfloat)6.75	* t1 * t0 * t0;
	dest[2] = (GLfloat)6.75	* t1 * t1 * t0;

	return;
}

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
            store_BGR_color(iter, context_ptr->pixels[cur_ind]);
		}
	}
}

#define FINAL_CODE

#ifndef NO_DRAWING

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

#endif

#undef FINAL_CODE

int run_Mandelbrot() {
	#define FINAL_CODE

	FILE *output_ptr = fopen("Test_CPF.csv", "w");
	if (!output_ptr) { CLEAR_RESOURCES(); return errno; }
	#undef FINAL_CODE
	#define FINAL_CODE	\
	fclose(output_ptr);

#ifndef NO_DRAWING

	glfwSetErrorCallback(error_callback);

	if (glfwInit() == GLFW_FALSE) { CLEAR_RESOURCES(); return glfwGetError(0); }
	#undef FINAL_CODE
	#define FINAL_CODE	\
	glfwTerminate();	\
	fclose(output_ptr);

	#define DEFAULT_WIN_W	800
	#define DEFAULT_WIN_H	600
	GLFWwindow *win = glfwCreateWindow(DEFAULT_WIN_W, DEFAULT_WIN_H, "", 0, 0);
	if (!win) { CLEAR_RESOURCES(); return glfwGetError(0); }
	#undef FINAL_CODE
	#define FINAL_CODE		\
	glfwDestroyWindow(win);	\
	glfwTerminate();		\
	fclose(output_ptr);

	glfwMakeContextCurrent(win);
	glfwSwapInterval(1);

#endif

	struct Mandelbrot_context context = {};

#ifdef NO_DRAWING

	#define DEFAULT_BUFF_W	1000
	#define DEFAULT_BUFF_H	750
	context.w			= DEFAULT_BUFF_W;
	context.h			= DEFAULT_BUFF_H;
	context.buff_size	= (size_t)(context.w * context.h) * sizeof(*context.pixels);

	context.pixels = malloc(context.buff_size);
	if (!context.pixels) { CLEAR_RESOURCES(); return errno; }
	#undef FINAL_CODE
	#define FINAL_CODE		\
	free(context.pixels);	\
	fclose(output_ptr);

	context.scale = DEFAULT_SCALE * powf(SCALE_MLT, TEST_SCALE_CNT);
	context.x_off = 0;
	context.y_off = 0;

#else

	glfwGetFramebufferSize(win, &context.w, &context.h);
	glViewport(0, 0, context.w, context.h);
	context.buff_size = (size_t)(context.w * context.h) * sizeof(*context.pixels);

	context.pixels = malloc(context.buff_size);
	if (!context.pixels) { CLEAR_RESOURCES(); return errno; }
	#undef FINAL_CODE
	#define FINAL_CODE		\
	free(context.pixels);	\
	glfwDestroyWindow(win);	\
	glfwTerminate();		\
	fclose(output_ptr);

#ifdef TESTING
	context.scale = DEFAULT_SCALE * powf(SCALE_MLT, TEST_SCALE_CNT);
#else
	context.scale = DEFAULT_SCALE;
#endif

	context.x_off = 0;
	context.y_off = 0;
	glfwSetWindowUserPointer(win, &context);
	glfwSetFramebufferSizeCallback(win, buff_resize_callback);
	glfwSetKeyCallback(win, keyboard_callback);

#endif

#ifdef TESTING

	#define NEED_SAMPLES 0x80
	size_t left_samples = NEED_SAMPLES;
	fprintf(output_ptr, "CPF\n");

#endif

	#define CYC_REFRESH_CNT ((size_t)0x40)
	size_t	frames_cnt		= 0,
			last_rep_cyc	= __rdtsc();

#ifdef NO_DRAWING

	while (1) {
		update_context(&context);
		frames_cnt++;

		if (frames_cnt >= CYC_REFRESH_CNT) {
			size_t	cur_cyc		= __rdtsc(),
					pass_cyc	= cur_cyc - last_rep_cyc;
			double	CPF			= (double)pass_cyc / (double)frames_cnt;

#ifdef TESTING

			fprintf(output_ptr, "%.2f\n", CPF);
			if (!(--left_samples)) { break; }

#else

			fprintf(stderr, "%zu frames in %zu cycles = %.2f CPF\n", frames_cnt, pass_cyc, CPF);

#endif
			frames_cnt		= 0;
			last_rep_cyc	= __rdtsc();
		}
	}

#else

#ifndef TESTING

	#define MAX_CPF_TITLE_LENGTH ((size_t)0x40)
	char CPF_title[MAX_CPF_TITLE_LENGTH] = {};

#endif

	glfwWaitEvents();
	while (!glfwWindowShouldClose(win)) {
		CHECK_FUNC(update_frame, win);
		frames_cnt++;

		if (frames_cnt >= CYC_REFRESH_CNT) {
			size_t	cur_cyc		= __rdtsc(),
					pass_cyc	= cur_cyc - last_rep_cyc;
			double	CPF			= (double)pass_cyc / (double)frames_cnt;

#ifdef TESTING

			fprintf(output_ptr, "%.2f\n", CPF);
			if (!(--left_samples)) { break; }

#else

			snprintf(CPF_title, MAX_CPF_TITLE_LENGTH, "%zu frames in %zu cycles = %.2f CPF\n", frames_cnt, pass_cyc, CPF);
			glfwSetWindowTitle(win, CPF_title);

#endif

			frames_cnt		= 0;
			last_rep_cyc	= __rdtsc();
		}

		glfwWaitEvents();
	}

#endif

	int glfw_error = glfwGetError(0);
	CLEAR_RESOURCES();
	return glfw_error == GLFW_NO_ERROR ? 0 : glfw_error;

	#undef FINAL_CODE
}
