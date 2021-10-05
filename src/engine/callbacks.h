#pragma once

#include <GLFW/glfw3.h>
#include <eggscript/egg.h>

void onWindowResize(GLFWwindow* window, int width, int height);
void onKeyPress(GLFWwindow* window, int key, int scanCode, int action, int mods);

namespace es {
	void defineCallbacks();
	esEntryPtr onKeyPress(esEnginePtr esEngine, unsigned int argc, esEntryPtr arguments);
};
