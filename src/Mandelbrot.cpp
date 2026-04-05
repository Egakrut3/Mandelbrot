#include <stdio.h>
#include <GLFW/glfw3.h>

// TODO - Make OpenGL error-handling

static size_t const	ScreenWidth     = 800,
				    ScreenHeight    = 600;

#define GLFW_FAILED_TO_INIT 1

static void glfw_error_callback(int err, char const *desc) {
	fprintf(stderr, "GLFW Error %d: %s\n", err, desc);
}

int run_Mandelbrot() {
	if (!glfwInit()) { return GLFW_FAILED_TO_INIT; }
	glfwSetErrorCallback(glfw_error_callback);

	GLFWwindow *window = glfwCreateWindow(ScreenWidth, ScreenHeight, "GLFW Test", nullptr, nullptr);
	if (!window) { glfwTerminate(); return 0; }
	glfwMakeContextCurrent(window);

	float r = 0.0f, g = 0.0f, b = 0.0f;
    float dr = 0.01f, dg = 0.015f, db = 0.02f;
    while (!glfwWindowShouldClose(window)) {
        // Изменяем цвет плавно
        r += dr; if (r > 1.0f || r < 0.0f) dr = -dr;
        g += dg; if (g > 1.0f || g < 0.0f) dg = -dg;
        b += db; if (b > 1.0f || b < 0.0f) db = -db;

        // 6️⃣ Устанавливаем цвет очистки
        glClearColor(r, g, b, 1.0f);

        // 7️⃣ Очищаем экран цветовым буфером
        glClear(GL_COLOR_BUFFER_BIT);

        // 8️⃣ Меняем буферы — показываем кадр
        glfwSwapBuffers(window);

        // 9️⃣ Обработка событий окна
        glfwPollEvents();
    }

	glfwDestroyWindow(window);

	glfwTerminate();
	return 0;
}
