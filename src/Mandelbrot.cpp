#include "Mandelbrot.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <cstdio>

// TODO - Make error handling
// TODO - Possible use BGRA

#define GLFW_FAILED_TO_INIT				1
#define GLFW_FAILED_TO_CREATE_WINDOW	2

static int callback_done = 0;
static void glfw_error_callback(int err, char const *desc) {
	fprintf(stderr, "GLFW Error %d: %s\n", err, desc);
}

static int	ScreenWidth		= 0,
			ScreenHeight	= 0;
static void get_ScreenSize_callback(GLFWwindow *window, int Width, int Height) {
	if (callback_done) { return; }

	ScreenWidth		= Width;
	ScreenHeight	= Height;
	callback_done	= 1;
}

static void wait_resize_callback(GLFWwindow *window, int Width, int Height) {
	if (callback_done) { return; }

	callback_done = 1;
}

#define StartWidth	800
#define StartHeight	600

int run_Mandelbrot() {
	if (!glfwInit()) { return GLFW_FAILED_TO_INIT; }
	glfwSetErrorCallback(glfw_error_callback);

	GLFWmonitor *monitor = glfwGetPrimaryMonitor();
	GLFWvidmode const *mode = glfwGetVideoMode(monitor);
	GLFWwindow *window = glfwCreateWindow(StartWidth, StartHeight, "GLFW Test", nullptr, nullptr);
	if (!window) { glfwTerminate(); return GLFW_FAILED_TO_CREATE_WINDOW; }

	GLFWframebuffersizefun prev_callback = glfwSetFramebufferSizeCallback(window, get_ScreenSize_callback);
	callback_done = 0;
	glfwMaximizeWindow(window);
	if (!callback_done) {
		glfwWaitEvents();
	}
	fprintf(stderr, "Width = %d, Height = %d\n", ScreenWidth, ScreenHeight);
	
	glfwSetWindowSizeLimits(window, GLFW_DONT_CARE, GLFW_DONT_CARE, ScreenWidth, ScreenHeight);
	glfwSetFramebufferSizeCallback(window, wait_resize_callback);
	callback_done = 0;
	glfwSetWindowSize(window, StartWidth, StartHeight);
	if (!callback_done) {
		glfwWaitEvents();
	}

	glfwSetFramebufferSizeCallback(window, prev_callback);

	unsigned char *pixels = nullptr;
	pixels = (typeof pixels)calloc(ScreenWidth * ScreenHeight * 4, sizeof *pixels);
	assert(pixels);

	glfwMakeContextCurrent(window);
	glEnable(GL_TEXTURE_2D);

	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	int CurWidth	= 0,
		CurHeight	= 0;
	glfwGetFramebufferSize(window, &CurWidth, &CurHeight);
	for (size_t y = 0; y < CurHeight; y++) {
		for (size_t x = 0; x < CurWidth; x++) {
			size_t ind = (y * CurWidth + x) * 4;
			pixels[ind + 0] = x & 0xFF;
			pixels[ind + 1] = y & 0xFF;
			pixels[ind + 2] = (x + y) & 0xFF;
			pixels[ind + 3] = 0xFF;
		}
	}
	glViewport(0, 0, CurWidth, CurHeight);
	glTexImage2D(	GL_TEXTURE_2D, 0, GL_RGBA8, CurWidth, CurHeight,
					0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	fprintf(stderr, "Width = %d, Height = %d\n", CurWidth, CurHeight);
	while (!glfwWindowShouldClose(window)) {
		int	NewWidth	= 0,
			NewHeight	= 0;
		glfwGetFramebufferSize(window, &NewWidth, &NewHeight);
		if (NewWidth != CurWidth or NewHeight != CurHeight) {
			CurWidth	= NewWidth;
			CurHeight	= NewHeight;

			for (size_t y = 0; y < CurHeight; y++) {
				for (size_t x = 0; x < CurWidth; x++) {
					size_t ind = (y * CurWidth + x) * 4;
					pixels[ind + 0] = x & 0xFF;
					pixels[ind + 1] = y & 0xFF;
					pixels[ind + 2] = (x + y) & 0xFF;
					pixels[ind + 3] = 0xFF;
				}
			}
			glViewport(0, 0, CurWidth, CurHeight);
			glTexImage2D(	GL_TEXTURE_2D, 0, GL_RGBA8, CurWidth, CurHeight,
							0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		}

		glBegin(GL_QUADS);
			glTexCoord2f(0, 0); glVertex2f(-1, -1);
			glTexCoord2f(1, 0); glVertex2f( 1, -1);
			glTexCoord2f(1, 1); glVertex2f( 1,  1);
			glTexCoord2f(0, 1); glVertex2f(-1,  1);
		glEnd();
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteTextures(1, &tex);
	free(pixels);
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
