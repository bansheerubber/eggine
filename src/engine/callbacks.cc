#include "callbacks.h"

#include <stdio.h>

void onWindowResize(GLFWwindow* window, int width, int height) {
	printf("resized to %ld %ld\n", width, height);
}
