#include "Mandelbrot_run.h"
#include <immintrin.h>

// #define NO_DRAWING

#if defined(NO_DRAWING)

#include <math.h>

#else

#include "Mandelbrot_GLFW.h"

#endif

#define FINAL_CODE

#if defined(MANDELBROT_PACKED)

#define MANDELBROT_ITER ((size_t)0x80)

#else

#define MANDELBROT_ITER ((size_t)0x100)

#endif
#define MANDELBROT_BORDER2	((GLfloat)4)
static void store_BGR_color(size_t iter, GLfloat dest[3]) {
	GLfloat	t1 = (GLfloat)iter / MANDELBROT_ITER,
		t0 = 1 - t1;
	dest[0] = 1		* t0 * t0 * t0;
	dest[1] = (GLfloat)6.75	* t1 * t0 * t0;
	dest[2] = (GLfloat)6.75	* t1 * t1 * t0;

	CLEAR_RESOURCES();
}

#if defined(MANDELBROT_PACKED)

static void update_pixels(struct Mandelbrot_context *restrict context_ptr) {
	assert(context_ptr);
	assert(context_ptr->buff_w % PACKED_CNT == 0);

	__m512	border2		= _mm512_set1_ps(MANDELBROT_BORDER2),
		prog		= _mm512_setr_ps(	0x0, 0x1, 0x2, 0x3,
							0x4, 0x5, 0x6, 0x7,
							0x8, 0x9, 0xA, 0xB,
							0xC, 0xD, 0xE, 0xF),
		x_inc		= _mm512_set1_ps(PACKED_CNT * context_ptr->scale),
		y_inc		= _mm512_set1_ps(context_ptr->scale),
		start_x		= _mm512_fmadd_ps(prog, y_inc, _mm512_set1_ps((GLfloat)(-context_ptr->w / 2) * context_ptr->scale + context_ptr->x_off)),
		cur_y		= _mm512_set1_ps((GLfloat)(-context_ptr->h / 2) * context_ptr->scale + context_ptr->y_off);
	__m512i	iter_inc	= _mm512_set1_epi32(1);
	for (GLsizei y_it = 0; y_it < context_ptr->h; y_it++, cur_y = _mm512_add_ps(cur_y, y_inc)) {
		__m512 cur_x = start_x;
		for (GLsizei x_it = 0; x_it < context_ptr->w; x_it += PACKED_CNT, cur_x = _mm512_add_ps(cur_x, x_inc)) {
			__m512		x0			= cur_x,
					x			= x0,
					y0			= cur_y,
					y			= y0;
			__m512i		iter		= _mm512_setzero_epi32();
			__mmask16	is_small	= 0xFF'FF;
			for (size_t i = 0; i < MANDELBROT_ITER && is_small; i++, iter = _mm512_mask_add_epi32(iter, is_small, iter, iter_inc)) {
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

			alignas(alignof(iter)) GLsizei iter_arr[PACKED_CNT];
			_mm512_store_epi32(iter_arr, iter);
			for (size_t i = 0; i < PACKED_CNT; i++) {
				size_t cur_ind = (size_t)(y_it * context_ptr->buff_w + x_it) + i;
				store_BGR_color((size_t)iter_arr[i], context_ptr->pixels[cur_ind]);
			}
		}
	}

	CLEAR_RESOURCES();
}

#else

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

static void update_pixels(struct Mandelbrot_context *context_ptr) {
	assert(context_ptr);
	
	for (GLsizei y_it = 0; y_it < context_ptr->h; y_it++) {
		for (GLsizei x_it = 0; x_it < context_ptr->w; x_it++) {
			struct Complex	z0	= (struct Complex){	(GLfloat)(x_it - context_ptr->w / 2) * context_ptr->scale + context_ptr->x_off,
									(GLfloat)(y_it - context_ptr->h / 2) * context_ptr->scale + context_ptr->y_off},
					z	= z0;

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

#endif

#undef FINAL_CODE

int Mandelbrot_run() {
	#define FINAL_CODE

	FILE *restrict output_ptr = fopen("Test_CPF.csv", "w"); if (!output_ptr) { PRINT_LINE(); CLEAR_RESOURCES(); return errno; } // TODO -
	#undef FINAL_CODE
	#define FINAL_CODE	\
	fclose(output_ptr);

	struct Mandelbrot_context context = {};

#if defined(NO_DRAWING)
	#define DEFAULT_BUFF_W	1000
	#define DEFAULT_BUFF_H	750
	context.w = DEFAULT_BUFF_W;
	context.h = DEFAULT_BUFF_H;

#if defined(MANDELBROT_PACKED)
	context.buff_w		= GET_PACKED_ALIGNED(context.w);
	context.buff_size	= (size_t)(context.buff_w * context.h) * sizeof(*context.pixels);
#else
	context.buff_size = (size_t)(context.w * context.h) * sizeof(*context.pixels);
#endif

	typeof(context.pixels) restrict const pixels = malloc(context.buff_size); if (!pixels) { PRINT_LINE(); CLEAR_RESOURCES(); return errno; }
	context.pixels = pixels;
	#undef FINAL_CODE
	#define FINAL_CODE		\
	free(context.pixels);		\
	context.pixels = nullptr;	\
	fclose(output_ptr);

	context.scale = DEFAULT_SCALE * powf(SCALE_MLT, TEST_SCALE_CNT);
	context.x_off = 0;
	context.y_off = 0;
#else
	GLFWwindow *restrict win = nullptr;
	CHECK_PROC(Mandelbrot_glfw_enter, &win, &context);
	#undef FINAL_CODE
	#define FINAL_CODE		\
	Mandelbrot_glfw_leave(win);	\
	fclose(output_ptr);
#endif

#if defined(TESTING)
	#define NEED_SAMPLES 0x100
	size_t left_samples = NEED_SAMPLES;
	fprintf(output_ptr, "CPF\n");
#endif

	size_t last_rep_cyc = 0;
	__asm__ volatile ("mfence" ::: "memory");
	last_rep_cyc = __rdtsc();

#if defined(NO_DRAWING)
	while(1) {
#else
	while (!Mandelbrot_glfw_finish(win)) {
#endif

		update_pixels(&context);

	#if !defined(NO_DRAWING)
		CHECK_PROC(Mandelbrot_glfw_refresh, win);
	#endif

		size_t cur_cyc = 0;
		__asm__ volatile ("mfence" ::: "memory");
		cur_cyc = __rdtsc();

		size_t	pass_cyc	= cur_cyc - last_rep_cyc;

	#if defined(TESTING)
		fprintf(output_ptr, "%zu\n", pass_cyc);
		if (!--left_samples) { break; }
	#else
		// fprintf(stderr, "%zu\n", pass_cyc);
	#endif

		__asm__ volatile ("mfence" ::: "memory");
		last_rep_cyc	= __rdtsc();
	}

	CLEAR_RESOURCES();
	return 0;

	#undef FINAL_CODE
}