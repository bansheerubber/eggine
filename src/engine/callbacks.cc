#include "callbacks.h"

#include <stdio.h>

#include "engine.h"
#include "../util/cloneString.h"

void es::defineCallbacks() {
	esEntryType keyPressArguments[2] = {ES_ENTRY_STRING, ES_ENTRY_NUMBER};
	esRegisterFunction(engine->eggscript, ES_ENTRY_INVALID, &es::onKeyPress, "onKeyPress", 2, keyPressArguments);

	esEntryType mousePressArguments[2] = {ES_ENTRY_NUMBER, ES_ENTRY_NUMBER};
	esRegisterFunction(engine->eggscript, ES_ENTRY_INVALID, &es::onMousePress, "onMousePress", 2, mousePressArguments);
	esRegisterFunction(engine->eggscript, ES_ENTRY_INVALID, &es::onMouseMove, "onMouseMove", 2, mousePressArguments);
	esRegisterFunction(engine->eggscript, ES_ENTRY_INVALID, &es::onAxisMove, "onAxisMove", 2, mousePressArguments);
}

// key string, action
esEntryPtr es::onKeyPress(esEnginePtr esEngine, unsigned int argc, esEntryPtr arguments) {
	if(argc != 2) {
		return nullptr;
	}
	
	int key = engine->keyToScancode[string(arguments[0].stringData)];
	int action = (int)arguments[1].numberData;

	#ifndef __switch__
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
	#endif

	return nullptr;
}

esEntryPtr es::onMousePress(esEnginePtr esEngine, unsigned int argc, esEntryPtr arguments) {
	if(argc != 2) {
		return nullptr;
	}
	
	int button = (int)arguments[0].numberData;
	int action = (int)arguments[1].numberData;

	#ifndef __switch__
	if(action == GLFW_PRESS) {
		vector<binds::Keybind> &binds = engine->buttonToMousebind[button];
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
		vector<binds::Keybind> &binds = engine->buttonToMousebind[button];
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
		vector<binds::Keybind> &binds = engine->buttonToMousebind[button];
		for(auto &bind: binds) {
			vector<GameObject*> presses = engine->bindHeldToGameObject[bind.bind];
			for(GameObject* object: presses) {
				engine->heldEvents.push_back(pair(object, bind.bind));
			}
		}
	}
	#endif

	return nullptr;
}

esEntryPtr es::onMouseMove(esEnginePtr esEngine, unsigned int argc, esEntryPtr arguments) {
	if(argc != 2) {
		return nullptr;
	}

	engine->mouse.x = arguments[0].numberData;
	engine->mouse.y = arguments[1].numberData;
	return nullptr;
}

esEntryPtr es::onAxisMove(esEnginePtr esEngine, unsigned int argc, esEntryPtr arguments) {
	if(argc != 2) {
		return nullptr;
	}

	int axis = arguments[0].numberData;
	double value = arguments[1].numberData;
	
	auto &binds = engine->axisToKeybind[axis];
	for(binds::Keybind &bind: binds) {
		for(GameObject* gameObject: engine->bindAxisToGameObject[bind.bind]) {
			gameObject->onAxis(bind.bind, value);
		}
	}

	return nullptr;
}

#ifndef __switch__
void onKeyPress(GLFWwindow* window, int key, int scanCode, int action, int mods) {
	esEntry arguments[2];
	arguments[0].type = ES_ENTRY_STRING;
	arguments[0].stringData = cloneString(engine->scancodeToKey[key].c_str());
	arguments[1].type = ES_ENTRY_NUMBER;
	arguments[1].numberData = action;
	esCallFunction(engine->eggscript, "onKeyPress", 2, arguments);
}

void onMousePress(GLFWwindow* window, int button, int action, int mods) {
	esEntry arguments[2];
	arguments[0].type = ES_ENTRY_NUMBER;
	arguments[0].numberData = button;
	arguments[1].type = ES_ENTRY_NUMBER;
	arguments[1].numberData = action;
	esCallFunction(engine->eggscript, "onMousePress", 2, arguments);
}

void onMouseMove(GLFWwindow* window, double x, double y) {
	esEntry arguments[2];
	arguments[0].type = ES_ENTRY_NUMBER;
	arguments[0].numberData = x;
	arguments[1].type = ES_ENTRY_NUMBER;
	arguments[1].numberData = y;
	esCallFunction(engine->eggscript, "onMouseMove", 2, arguments);
}
#endif

void onAxisMove(binds::Axes axis, double value) {
	esEntry arguments[2];
	arguments[0].type = ES_ENTRY_NUMBER;
	arguments[0].numberData = axis;
	arguments[1].type = ES_ENTRY_NUMBER;
	arguments[1].numberData = value;
	esCallFunction(engine->eggscript, "onAxisMove", 2, arguments);
}
