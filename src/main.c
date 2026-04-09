#include "Common.h"
#include "Mandelbrot.h"
#include "Slow_Mandelbrot.h"

int main() {
	#define FINAL_CODE

	CHECK_FUNC(run_Mandelbrot);

	// CHECK_FUNC(run_Slow_Mandelbrot);

	return 0;

	#undef FINAL_CODE
}