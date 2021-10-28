#pragma once

#ifndef __switch__
#include <GLFW/glfw3.h>
#endif

#include <eggscript/egg.h>

#include "keybind.h"

// void onWindowResize(GLFWwindow* window, int width, int height);

#ifndef __switch__
void onKeyPress(GLFWwindow* window, int key, int scanCode, int action, int mods);
void onMousePress(GLFWwindow* window, int button, int action, int mods);
void onMouseMove(GLFWwindow* window, double x, double y);
#endif

void onAxisMove(binds::Axes axis, double value);
void onGamepadButton(binds::GamepadButtons bind, bool pressed);

namespace es {
	void defineCallbacks();
	esEntryPtr onKeyPress(esEnginePtr esEngine, unsigned int argc, esEntryPtr arguments);
	esEntryPtr onMousePress(esEnginePtr esEngine, unsigned int argc, esEntryPtr arguments);
	esEntryPtr onAxisMove(esEnginePtr esEngine, unsigned int argc, esEntryPtr arguments);
	esEntryPtr onGamepadButton(esEnginePtr esEngine, unsigned int argc, esEntryPtr arguments);
};
