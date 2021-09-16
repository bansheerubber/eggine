#include "callbacks.h"

#include <stdio.h>

void onWindowResize(GLFWwindow* window, int width, int height) {
	engine->windowWidth = width;
	engine->windowHeight = height;

	glViewport(0, 0, width, height);
}

void onKeyPress(GLFWwindow* window, int key, int scanCode, int action, int mods) {
	if(action == GLFW_PRESS) {
		vector<binds::Keybind> &binds = engine->keyToKeybind[key];
		for(auto &bind: binds) {
			vector<GameObject*> presses = engine->bindPressToGameObject[bind.bind];
			for(GameObject* object: presses) {
				object->onBindPress(bind.bind);
			}
		}
	}
}
