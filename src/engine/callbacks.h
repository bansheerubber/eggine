#pragma once

#include <glfw/glfw3.h>
#include <torquescript/ts.h>

void onWindowResize(GLFWwindow* window, int width, int height);
void onKeyPress(GLFWwindow* window, int key, int scanCode, int action, int mods);

namespace ts {
	void defineCallbacks();
	tsEntryPtr onKeyPress(tsEnginePtr tsEngine, unsigned int argc, tsEntryPtr arguments);
};
