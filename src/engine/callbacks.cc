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
	else if(action == GLFW_RELEASE) {
		vector<binds::Keybind> &binds = engine->keyToKeybind[key];
		for(auto &bind: binds) {
			vector<GameObject*> presses = engine->bindReleaseToGameObject[bind.bind];
			for(GameObject* object: presses) {
				object->onBindRelease(bind.bind);
			}
		}
	}
	else if(action == GLFW_REPEAT) {
		// add to engine list so it can process this during the next/current tick
		vector<binds::Keybind> &binds = engine->keyToKeybind[key];
		for(auto &bind: binds) {
			vector<GameObject*> presses = engine->bindHeldToGameObject[bind.bind];
			for(GameObject* object: presses) {
				engine->heldEvents.push_back(pair(object, bind.bind));
			}
		}
	}
}
