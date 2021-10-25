#pragma once

#ifndef __switch__
#include <GLFW/glfw3.h>
#endif

#include <eggscript/egg.h>

// void onWindowResize(GLFWwindow* window, int width, int height);

#ifndef __switch__
void onKeyPress(GLFWwindow* window, int key, int scanCode, int action, int mods);
#endif

namespace es {
	void defineCallbacks();
	esEntryPtr onKeyPress(esEnginePtr esEngine, unsigned int argc, esEntryPtr arguments);
};
