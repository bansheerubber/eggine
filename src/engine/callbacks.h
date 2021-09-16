#pragma once

#include <glfw/glfw3.h>

#include "engine.h"

void onWindowResize(GLFWwindow* window, int width, int height);
void onKeyPress(GLFWwindow* window, int key, int scanCode, int action, int mods);
