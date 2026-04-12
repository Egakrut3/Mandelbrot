#include "Mandelbrot.h"
#include <GLFW/glfw3.h>
#include <immintrin.h>

// #define NO_DRAWING

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
	size_t	size;
	GLsizei	w,
			h;
	GLfloat	(*pixels)[3],
			scale,
			x_off,
			y_off;
};

#define PACKED_SIZE	0x40
static GLsizei get_packed_aligned(GLsizei size) {
	return (size + (PACKED_SIZE - 1)) & ~(PACKED_SIZE - 1);
}

static void buff_resize_callback(GLFWwindow *win, int w, int h) {
	assert(win);

	struct Mandelbrot_context *context_ptr = glfwGetWindowUserPointer(win);
	assert(context_ptr);

	glViewport(0, 0, w, h); // I intentionally pass real width, not buffer width
	context_ptr->w = get_packed_aligned(w);
	context_ptr->h = h;
	glPixelStorei(GL_UNPACK_ROW_LENGTH, context_ptr->w);
	size_t new_size = (size_t)(context_ptr->w * context_ptr->h) * sizeof(*context_ptr->pixels);

	if (new_size > context_ptr->size) {
		context_ptr->size = max(context_ptr->size * 2, new_size);
		void *new_pixels = realloc(context_ptr->pixels, context_ptr->size);
		if (!new_pixels) { PRINT_LINE(); return; } // I can't handle it, because glfwTerminate is not reenterable
		context_ptr->pixels = new_pixels;
	}
}

#define DEFAULT_SCALE	((GLfloat)0.003)
#define SCALE_MLT		((GLfloat)1.1)
#define PIXEL_STEP		160
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

#define MANDELBROT_ITER	((size_t) 32)
static void store_BGR_color(GLsizei iter, GLfloat dest[3]) {
	GLfloat	t1 = (GLfloat)iter / MANDELBROT_ITER,
			t0 = 1 - t1;
	dest[0] = 1				* t0 * t0 * t0;
	dest[1] = (GLfloat)6.75	* t1 * t0 * t0;
	dest[2] = (GLfloat)6.75	* t1 * t1 * t0;

	return;
}

#define MANDELBROT_BORDER2	((GLfloat)4)
#define PACKED_CNT			((GLsizei)(PACKED_SIZE / sizeof(GLfloat)))
static_assert(PACKED_SIZE % sizeof(GLfloat) == 0);
static void update_context(struct Mandelbrot_context *context_ptr) {
	assert(context_ptr);

	__m512	border2		=	_mm512_set1_ps(MANDELBROT_BORDER2),
			prog		=	_mm512_setr_ps(	0,		1,		2,		3,
											4,		5,		6,		7,
											8,		9,		10,		11,
											12,		13,		14,		15),
			x_inc		=	_mm512_set1_ps(PACKED_CNT * context_ptr->scale),
			y_inc		=	_mm512_set1_ps(context_ptr->scale),
			start_x 	=	_mm512_fmadd_ps(prog, y_inc,
							_mm512_set1_ps((GLfloat)(-context_ptr->w / 2) * context_ptr->scale + context_ptr->x_off)),
			cur_y		=	_mm512_set1_ps((GLfloat)(-context_ptr->h / 2) * context_ptr->scale + context_ptr->y_off);
	__m512i iter_inc	=	_mm512_set1_epi32(1);
	
	for (GLsizei y_it = 0; y_it < context_ptr->h; y_it++, cur_y = _mm512_add_ps(cur_y, y_inc)) {
		__m512 cur_x = start_x;
		for (GLsizei x_it = 0; x_it < context_ptr->w; x_it += PACKED_CNT, cur_x = _mm512_add_ps(cur_x, x_inc)) {
			__m512	x0	= cur_x,
					x	= x0,
					y0	= cur_y,
					y	= y0;
			__m512i	iter = _mm512_setzero_epi32();
			__mmask16 is_small = 0xFF'FF;

			for (size_t i = 0; i < MANDELBROT_ITER and is_small; i++, iter = _mm512_mask_add_epi32(iter, is_small, iter, iter_inc)) {
				__m512 abs2 = _mm512_mul_ps(x, x);
				abs2 = _mm512_fmadd_ps(y, y, abs2);
				__mmask16 new_mask = _mm512_cmp_ps_mask(abs2, border2, _CMP_LE_OQ);
				is_small &= new_mask;

				__m512	new_x = _mm512_fmadd_ps(x, x, x0),
						new_y = _mm512_fmadd_ps(x, y, y0);
				new_x = _mm512_fnmadd_ps(y, y, new_x);
				new_y = _mm512_fmadd_ps(x, y, new_y);

				x = _mm512_fmadd_ps(new_x, new_x, x0);	// Two steps at a time!
				y = _mm512_fmadd_ps(new_x, new_y, y0);
				x = _mm512_fnmadd_ps(new_y, new_y, x);
				y = _mm512_fmadd_ps(new_x, new_y, y);
			}

			_Alignas(PACKED_SIZE) GLsizei iter_arr[PACKED_CNT];
			_mm512_store_epi32(iter_arr, iter);
			for (size_t i = 0; i < PACKED_CNT; i++) {
				size_t cur_ind = (size_t)(y_it * context_ptr->w + x_it) + i;
				store_BGR_color(iter_arr[i], context_ptr->pixels[cur_ind]);
			}
		}
	}
}

#define FINAL_CODE

static int update_frame(GLFWwindow *win) {
	assert(win);

	struct Mandelbrot_context *context_ptr = glfwGetWindowUserPointer(win);
	update_context(context_ptr); // TODO - Disimprovement

#ifndef NO_DRAWING

	GL_CALL(glDrawPixels, context_ptr->w, context_ptr->h, GL_BGR, GL_FLOAT, context_ptr->pixels);

#endif

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

	GLFWwindow *win = glfwCreateWindow(DEFAULT_WIN_W, DEFAULT_WIN_H, "", 0, 0);
	if (!win) { CLEAR_RESOURCES(); return glfwGetError(0); }
	#undef FINAL_CODE
	#define FINAL_CODE		\
	glfwDestroyWindow(win);	\
	glfwTerminate();

	glfwMakeContextCurrent(win);
	glfwSwapInterval(1);

	struct Mandelbrot_context context = {};
	glfwGetFramebufferSize(win, &context.w, &context.h);
	glViewport(0, 0, context.w, context.h); // I intentionally pass real width, not buffer width
	context.w = get_packed_aligned(context.w);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, context.w);
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
	glfwSetFramebufferSizeCallback(win, buff_resize_callback);
	glfwSetKeyCallback(win, keyboard_callback);

	#define MAX_FPS_TITLE_LENGTH	((size_t)0x40)
	#define FPS_REFRESH_TIME		((double)5)
	size_t	frames_cnt						= 0;
	double	last_FPS_rep_time				= glfwGetTime();
	char	FPS_title[MAX_FPS_TITLE_LENGTH]	= "";
	while (!glfwWindowShouldClose(win)) {
		CHECK_FUNC(update_frame, win);
		frames_cnt++;

		double	cur_time	= glfwGetTime(),
				pass_time	= cur_time - last_FPS_rep_time;
		if (pass_time >= FPS_REFRESH_TIME) {
			snprintf(FPS_title, MAX_FPS_TITLE_LENGTH, "%zu frames in %.1f seconds = %.2f FPS", frames_cnt, FPS_REFRESH_TIME, (double) frames_cnt / pass_time);
			glfwSetWindowTitle(win, FPS_title);
			frames_cnt = 0;
			last_FPS_rep_time = cur_time;
		}

		glfwWaitEvents();
	}

	int glfw_error = glfwGetError(0);
	CLEAR_RESOURCES();
	return glfw_error == GLFW_NO_ERROR ? 0 : glfw_error;

	#undef FINAL_CODE
}
