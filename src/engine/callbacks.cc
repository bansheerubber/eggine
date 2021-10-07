#include "../helpers.h"
#include GLAD_HEADER

#include "callbacks.h"

#include <stdio.h>

#include "engine.h"
#include "../util/cloneString.h"

void es::defineCallbacks() {
	esEntryType keyPressArguments[2] = {ES_ENTRY_STRING, ES_ENTRY_NUMBER};
	esRegisterFunction(engine->eggscript, ES_ENTRY_INVALID, &es::onKeyPress, "onKeyPress", 2, keyPressArguments);
}

void onWindowResize(GLFWwindow* window, int width, int height) {
	engine->windowWidth = width;
	engine->windowHeight = height;

	glViewport(0, 0, width, height);
}

// key string, action
esEntryPtr es::onKeyPress(esEnginePtr esEngine, unsigned int argc, esEntryPtr arguments) {
	if(argc != 2) {
		return nullptr;
	}
	
	int key = engine->keyToScancode[string(arguments[0].stringData)];
	int action = (int)arguments[1].numberData;

	if(action == GLFW_PRESS) {
		vector<binds::Keybind> &binds = engine->keyToKeybind[key];
		for(auto &bind: binds) {
			vector<GameObject*> presses = engine->bindPressToGameObject[bind.bind];
			for(GameObject* object: presses) {
				object->onBindPress(bind.bind);
			}

			// handle TS callbacks
			vector<string> esPresses = engine->bindToTSCallback[bind.bind];
			for(string &callback: esPresses) {
				esEntry arguments[1];
				arguments[0].type = ES_ENTRY_NUMBER;
				arguments[0].numberData = 1;
				esCallFunction(engine->eggscript, callback.c_str(), 1, arguments);
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

			// handle TS callbacks
			vector<string> esPresses = engine->bindToTSCallback[bind.bind];
			for(string &callback: esPresses) {
				esEntry arguments[1];
				arguments[0].type = ES_ENTRY_NUMBER;
				arguments[0].numberData = 0;
				esCallFunction(engine->eggscript, callback.c_str(), 1, arguments);
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

	return nullptr;
}

void onKeyPress(GLFWwindow* window, int key, int scanCode, int action, int mods) {
	esEntry arguments[2];
	arguments[0].type = ES_ENTRY_STRING;
	arguments[0].stringData = cloneString(engine->scancodeToKey[key].c_str());
	arguments[1].type = ES_ENTRY_NUMBER;
	arguments[1].numberData = action;
	esCallFunction(engine->eggscript, "onKeyPress", 2, arguments);
}
