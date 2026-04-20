#include "Mandelbrot_GLFW.h"

#if defined(TESTING)

#include <math.h>

#endif

#define MANDELBROT_PACKED

#define GLFW_CHECK()								\
do {										\
	char const *str = nullptr;						\
	int __cur_err_val = glfwGetError(&str);					\
	if (__cur_err_val == GLFW_NO_ERROR) { break; }				\
	ON_DEBUG(								\
		fprintf(stderr, "Error with code %d found\n", __cur_err_val);	\
		PRINT_LINE();							\
		fputs(str, stderr);						\
		putc('\n', stderr);						\
	);									\
	CLEAR_RESOURCES();							\
	return __cur_err_val;							\
} while (false)

#define FINAL_CODE

static void error_callback(int const err, char const *const desc) {
	assert(desc);

	fprintf(stderr, "GLFW Error %d: %s\n", err, desc);

	CLEAR_RESOURCES();
}

static void buff_resize_callback(GLFWwindow *restrict const win, int const w, int const h) {
	assert(win);

	struct Mandelbrot_context *restrict const context = glfwGetWindowUserPointer(win); assert(glfwGetError(nullptr) == GLFW_NO_ERROR);
	assert(context);

	context->w = w;
	context->h = h;

	glViewport(0, 0, context->w, context->h); assert(glGetError() == GL_NO_ERROR);

#if defined(MANDELBROT_PACKED)
	context->buff_w = GET_PACKED_ALIGNED(context->w);

	glPixelStorei(GL_UNPACK_ROW_LENGTH, context->buff_w); assert(glGetError() == GL_NO_ERROR);

	size_t const new_buff_size = (size_t)(context->buff_w * context->h) * sizeof(*context->pixels);
#else
	size_t const new_buff_size = (size_t)(context->w * context->h) * sizeof(*context->pixels);
#endif

	if (new_buff_size > context->buff_size) {
		context->buff_size = max(context->buff_size * 2, new_buff_size); // TODO -

		void *restrict const new_pixels = realloc(context->pixels, context->buff_size); if (!new_pixels) { PRINT_LINE(); CLEAR_RESOURCES(); return; }

		context->pixels = new_pixels;
	}

	CLEAR_RESOURCES();
}

#define PIXEL_STEP ((GLfloat)160)
static void keyboard_callback(GLFWwindow *restrict const win, int const key, [[maybe_unused]] int const scancode, int const action, int const mods) {
	assert(win);

	struct Mandelbrot_context *restrict const context = glfwGetWindowUserPointer(win); assert(glfwGetError(nullptr) == GLFW_NO_ERROR);
	assert(context);

	if (action == GLFW_PRESS) {
		if (mods & GLFW_MOD_SHIFT) {
			if (key == GLFW_KEY_EQUAL) { context->scale /= SCALE_MLT; }
			if (key == GLFW_KEY_MINUS) { context->scale *= SCALE_MLT; }
		}

		if (key == GLFW_KEY_RIGHT)	{ context->x_off += context->scale * PIXEL_STEP; }
		if (key == GLFW_KEY_LEFT)	{ context->x_off -= context->scale * PIXEL_STEP; }
		if (key == GLFW_KEY_UP)		{ context->y_off += context->scale * PIXEL_STEP; }
		if (key == GLFW_KEY_DOWN)	{ context->y_off -= context->scale * PIXEL_STEP; }
	}

	CLEAR_RESOURCES();
}

#undef FINAL_CODE

int Mandelbrot_glfw_enter(GLFWwindow *restrict *restrict const win_ptr, struct Mandelbrot_context *restrict const context) {
	#define FINAL_CODE

	assert(win_ptr); assert(context);

	glfwSetErrorCallback(error_callback);

	glfwInit(); GLFW_CHECK();
	#undef FINAL_CODE
	#define FINAL_CODE	\
	glfwTerminate();

	#define DEFAULT_WIN_W 800
	#define DEFAULT_WIN_H 600
	*win_ptr = glfwCreateWindow(DEFAULT_WIN_W, DEFAULT_WIN_H, "", nullptr, nullptr); GLFW_CHECK();
	#undef FINAL_CODE
	#define FINAL_CODE		\
	glfwDestroyWindow(*win_ptr);	\
	glfwTerminate();

	glfwMakeContextCurrent(*win_ptr); GLFW_CHECK();

	glfwGetFramebufferSize(*win_ptr, &context->w, &context->h); GLFW_CHECK();

	glViewport(0, 0, context->w, context->h); assert(glGetError() == GL_NO_ERROR);

#if defined(MANDELBROT_PACKED)
	context->buff_w = GET_PACKED_ALIGNED(context->w);

	glPixelStorei(GL_UNPACK_ROW_LENGTH, context->buff_w); assert(glGetError() == GL_NO_ERROR);

	context->buff_size = (size_t)(context->buff_w * context->h) * sizeof(*context->pixels);
#else
	context->buff_size = (size_t)(context->w * context->h) * sizeof(*context->pixels);
#endif

	typeof(context->pixels) restrict const pixels = malloc(context->buff_size); if (!pixels) { PRINT_LINE(); CLEAR_RESOURCES(); return errno; }
	context->pixels = pixels;
	#undef FINAL_CODE
	#define FINAL_CODE		\
	free(context->pixels);		\
	context->pixels = nullptr;	\
	glfwDestroyWindow(*win_ptr);	\
	glfwTerminate();

#if defined(TESTING)
	context->scale = DEFAULT_SCALE * powf(SCALE_MLT, TEST_SCALE_CNT);
#else
	context->scale = DEFAULT_SCALE;
#endif

	context->x_off = 0;
	context->y_off = 0;

	glfwSetWindowUserPointer(*win_ptr, context);
	glfwSetFramebufferSizeCallback(*win_ptr, buff_resize_callback);
	glfwSetKeyCallback(*win_ptr, keyboard_callback);
	assert(glfwGetError(nullptr) == GLFW_NO_ERROR);

	glfwSwapInterval(1); GLFW_CHECK();

	return 0;

	#undef FINAL_CODE
}

#define FINAL_CODE

int Mandelbrot_glfw_finish(GLFWwindow *const win) {
	int ans = glfwWindowShouldClose(win); assert(glfwGetError(nullptr) == GLFW_NO_ERROR);
	CLEAR_RESOURCES();
	return ans;
}

int Mandelbrot_glfw_refresh(GLFWwindow *restrict const win) {
	assert(win);

	struct Mandelbrot_context *restrict const context = glfwGetWindowUserPointer(win); assert(glfwGetError(nullptr) == GLFW_NO_ERROR);

	typeof(context->pixels) restrict const pixels = context->pixels;
	glDrawPixels(context->w, context->h, GL_BGR, GL_FLOAT, pixels); assert(glGetError() == GL_NO_ERROR);

	glfwSwapBuffers(win); GLFW_CHECK();
	glfwWaitEvents(); GLFW_CHECK();

	CLEAR_RESOURCES();
	return 0;
}

void Mandelbrot_glfw_leave(GLFWwindow *restrict const win) {
	assert(win);

	struct Mandelbrot_context *restrict const context = glfwGetWindowUserPointer(win); assert(glfwGetError(nullptr) == GLFW_NO_ERROR);

	typeof(context->pixels) restrict const pixels = context->pixels;
	free(pixels);
	context->pixels = nullptr;

	glfwDestroyWindow(win);
	glfwTerminate();

	CLEAR_RESOURCES();
}

#undef FINAL_CODE