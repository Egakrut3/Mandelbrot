#ifndef MANDELBROT_COMMON
#define MANDELBROT_COMMON

#include "Common.h"
#include <GL/gl.h>

#define TESTING
// #define MANDELBROT_PACKED

#define DEFAULT_SCALE	((GLfloat)0.003)
#define TEST_SCALE_CNT	((GLfloat)-0x20)

#if defined(MANDELBROT_PACKED)

#define PACKED_SIZE	((size_t)0x40)
#define PACKED_CNT	((GLsizei)(PACKED_SIZE / sizeof(GLfloat)))
static_assert(PACKED_SIZE % sizeof(GLfloat) == 0);
#define GET_PACKED_ALIGNED(size) (((size_t)(size) + PACKED_SIZE - 1) & ~(PACKED_SIZE - 1))

#endif

struct Mandelbrot_context {
	size_t	buff_size;
	GLfloat	(*pixels)[3];	// BGR format
	GLsizei	w,
		h;
#if defined(MANDELBROT_PACKED)
	GLsizei	buff_w;
#endif

	GLfloat	scale,
		x_off,
		y_off;
};

#define SCALE_MLT ((GLfloat)1.1)

#endif