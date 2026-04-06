#include "Mandelbrot.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>

// TODO - Make error handling
// TODO - Possible use BGRA

static size_t const	ScreenWidth		= 2560,
					ScreenHeight	= 1600;

#define GLFW_FAILED_TO_INIT				1
#define GLFW_FAILED_TO_CREATE_WINDOW	2

static void glfw_error_callback(int err, char const *desc) {
	fprintf(stderr, "GLFW Error %d: %s\n", err, desc);
}

int run_Mandelbrot() {
	if (!glfwInit()) { return GLFW_FAILED_TO_INIT; }
	glfwSetErrorCallback(glfw_error_callback);

	GLFWmonitor *monitor = glfwGetPrimaryMonitor();
	GLFWvidmode const *mode = glfwGetVideoMode(monitor);
	GLFWwindow *window = glfwCreateWindow(800, 600, "GLFW Test", nullptr, nullptr);
	if (!window) { glfwTerminate(); return GLFW_FAILED_TO_CREATE_WINDOW; }
	glfwMaximizeWindow(window);
	glfwMakeContextCurrent(window);

	int BuffWidth	= 0,
		BuffHeight	= 0;
    glfwGetFramebufferSize(window, &BuffWidth, &BuffHeight);
	size_t BuffSize = BuffWidth * BuffHeight * 4;
    unsigned char *pixels = nullptr;
	pixels = (typeof pixels)calloc(BuffSize, sizeof *pixels);
    assert(pixels);

	GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(	GL_TEXTURE_2D, 0, GL_RGBA8, BuffWidth, BuffHeight,
					0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	while (!glfwWindowShouldClose(window)) {
		int	BuffWidth	= 0,
			BuffHeight	= 0;
		glfwGetFramebufferSize(window, &BuffWidth, &BuffHeight);
		size_t NewBuffSize = BuffWidth * BuffHeight * 4;
		if (NewBuffSize != BuffSize) { // TODO
			BuffSize = NewBuffSize;
			pixels = (typeof pixels)realloc(pixels, BuffSize * sizeof *pixels);
			assert(pixels);

			glTexImage2D(	GL_TEXTURE_2D, 0, GL_RGBA8, BuffWidth, BuffHeight,
							0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		}

		for (size_t y = 0; y < BuffHeight; y++) {
            for (size_t x = 0; x < BuffWidth; x++) {
				size_t ind = (y * BuffWidth + x) * 4;
                pixels[ind + 0] = x & 0xFF;
                pixels[ind + 1] = y & 0xFF;
                pixels[ind + 2] = (x + y) & 0xFF;
				pixels[ind + 3] = 0xFF;
            }
        }
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, BuffWidth, BuffHeight,
						GL_RGBA, GL_UNSIGNED_BYTE, pixels);

		glEnable(GL_TEXTURE_2D);
		glViewport(0, 0, BuffWidth, BuffHeight);
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
