#include "callbacks.h"

#include <stdio.h>

#include "engine.h"
#include "../util/cloneString.h"

void ts::defineCallbacks() {
	tsEntryType keyPressArguments[2] = {TS_ENTRY_STRING, TS_ENTRY_NUMBER};
	tsRegisterFunction(engine->torquescript, TS_ENTRY_INVALID, &ts::onKeyPress, "onKeyPress", 2, keyPressArguments);
}

void onWindowResize(GLFWwindow* window, int width, int height) {
	engine->windowWidth = width;
	engine->windowHeight = height;

	glViewport(0, 0, width, height);
}

// key string, action
tsEntryPtr ts::onKeyPress(tsEnginePtr tsEngine, unsigned int argc, tsEntryPtr arguments) {
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
			vector<string> tsPresses = engine->bindToTSCallback[bind.bind];
			for(string &callback: tsPresses) {
				tsEntry arguments[1];
				arguments[0].type = TS_ENTRY_NUMBER;
				arguments[0].numberData = 1;
				tsCallFunction(engine->torquescript, callback.c_str(), 1, arguments);
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
			vector<string> tsPresses = engine->bindToTSCallback[bind.bind];
			for(string &callback: tsPresses) {
				tsEntry arguments[1];
				arguments[0].type = TS_ENTRY_NUMBER;
				arguments[0].numberData = 0;
				tsCallFunction(engine->torquescript, callback.c_str(), 1, arguments);
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
	tsEntry arguments[2];
	arguments[0].type = TS_ENTRY_STRING;
	arguments[0].stringData = cloneString(engine->scancodeToKey[key].c_str());
	arguments[1].type = TS_ENTRY_NUMBER;
	arguments[1].numberData = action;
	tsCallFunction(engine->torquescript, "onKeyPress", 2, arguments);
}
