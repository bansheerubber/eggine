#include "callbacks.h"

#include <stdio.h>

void onWindowResize(GLFWwindow* window, int width, int height) {
	engine->windowWidth = width;
	engine->windowHeight = height;

	glViewport(0, 0, width, height);
}
