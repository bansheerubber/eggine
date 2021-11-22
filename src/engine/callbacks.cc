#include "callbacks.h"

#include "../engine/developer.h"

#include <stdio.h>

#include "engine.h"
#include "../util/cloneString.h"

void es::defineCallbacks() {
	esEntryType keyPressArguments[2] = {ES_ENTRY_STRING, ES_ENTRY_NUMBER};
	esRegisterFunction(engine->eggscript, ES_ENTRY_INVALID, &es::onKeyPress, "onKeyPress", 2, keyPressArguments);

	esEntryType mousePressArguments[2] = {ES_ENTRY_NUMBER, ES_ENTRY_NUMBER};
	esRegisterFunction(engine->eggscript, ES_ENTRY_INVALID, &es::onMousePress, "onMousePress", 2, mousePressArguments);
	esRegisterFunction(engine->eggscript, ES_ENTRY_INVALID, &es::onAxisMove, "onAxisMove", 2, mousePressArguments);
	esRegisterFunction(engine->eggscript, ES_ENTRY_INVALID, &es::onGamepadButton, "onGamepadButton", 2, mousePressArguments);
}

// key string, action
esEntryPtr es::onKeyPress(esEnginePtr esEngine, unsigned int argc, esEntryPtr arguments) {
	if(argc != 2) {
		return nullptr;
	}
	
	int key = engine->keyToScancode[string(arguments[0].stringData)];
	int action = (int)arguments[1].numberData;

	#ifndef __switch__
	#ifdef EGGINE_DEVELOPER_MODE
	if(ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow)) {
		return nullptr;
	}
	#endif

	if(action != GLFW_REPEAT) {
		vector<binds::Keybind> &binds = engine->keyToKeybind[key];
		for(auto &bind: binds) {
			tsl::robin_set<GameObject*> presses = engine->bindToGameObject[bind.bind];
			for(GameObject* object: presses) {
				object->onBind(bind.bind, action == GLFW_RELEASE ? binds::RELEASE : binds::PRESS);
			}

			// handle TS callbacks
			tsl::robin_set<string> esPresses = engine->bindToTSCallback[bind.bind];
			for(auto &callback: esPresses) {
				esEntry arguments[1];
				arguments[0].type = ES_ENTRY_NUMBER;
				arguments[0].numberData = action;
				esCallFunction(engine->eggscript, callback.c_str(), 1, arguments);
			}

			// handle TS object callbacks
			tsl::robin_set<pair<esObjectReferencePtr, string>> esObjectPresses = engine->bindToTSObjectCallback[bind.bind];
			for(auto &[object, callback]: esObjectPresses) {
				if(object->objectWrapper != nullptr) { // TODO delete from vector if object was deleted
					esEntry arguments[2];
					esCreateObjectAt(&arguments[0], object);
					esCreateNumberAt(&arguments[1], action);
					esCallMethod(engine->eggscript, object, callback.c_str(), 2, arguments);
				}
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

	if(action == GLFW_PRESS) {
		litehtml::position::vector redraw;
		engine->renderWindow.htmlDocument->on_lbutton_down(engine->mouse.x, engine->mouse.y, engine->mouse.x, engine->mouse.y, redraw);
	}
	else if(action == GLFW_RELEASE) {
		litehtml::position::vector redraw;
		engine->renderWindow.htmlDocument->on_lbutton_up(engine->mouse.x, engine->mouse.y, engine->mouse.x, engine->mouse.y, redraw);
	}

	#ifndef __switch__
	#ifdef EGGINE_DEVELOPER_MODE
	if(ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow) || ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
		return nullptr;
	}
	#endif

	if(action != GLFW_REPEAT) {
		// handle game objects
		vector<binds::Keybind> &binds = engine->buttonToMousebind[button];
		for(auto &bind: binds) {
			tsl::robin_set<GameObject*> presses = engine->bindToGameObject[bind.bind];
			for(GameObject* object: presses) {
				object->onBind(bind.bind, action == GLFW_RELEASE ? binds::RELEASE : binds::PRESS);
			}

			// handle TS callbacks
			tsl::robin_set<string> esPresses = engine->bindToTSCallback[bind.bind];
			for(auto &callback: esPresses) {
				esEntry arguments[1];
				arguments[0].type = ES_ENTRY_NUMBER;
				arguments[0].numberData = action;
				esCallFunction(engine->eggscript, callback.c_str(), 1, arguments);
			}

			// handle TS object callbacks
			tsl::robin_set<pair<esObjectReferencePtr, string>> esObjectPresses = engine->bindToTSObjectCallback[bind.bind];
			for(auto &[object, callback]: esObjectPresses) {
				if(object->objectWrapper != nullptr) { // TODO delete from vector if object was deleted
					esEntry arguments[2];
					esCreateObjectAt(&arguments[0], object);
					esCreateNumberAt(&arguments[1], action);
					esCallMethod(engine->eggscript, object, callback.c_str(), 2, arguments);
				}
			}
		}
	}
	#endif

	return nullptr;
}

esEntryPtr es::onAxisMove(esEnginePtr esEngine, unsigned int argc, esEntryPtr arguments) {
	if(argc != 2) {
		return nullptr;
	}

	binds::Axes axis = (binds::Axes)arguments[0].numberData;
	double value = arguments[1].numberData;

	if(axis == binds::MOUSE_AXIS_X) {
		engine->mouse.x = value;
	}
	else if(axis == binds::MOUSE_AXIS_Y) {
		engine->mouse.y = value;
	}
	
	auto &binds = engine->axisToKeybind[axis];
	for(binds::Keybind &bind: binds) {
		for(GameObject* gameObject: engine->bindAxisToGameObject[bind.bind]) {
			gameObject->onAxis(bind.bind, value);
		}
	}

	return nullptr;
}

esEntryPtr es::onGamepadButton(esEnginePtr esEngine, unsigned int argc, esEntryPtr arguments) {
	if(argc != 2) {
		return nullptr;
	}
	
	binds::GamepadButtons button = (binds::GamepadButtons)arguments[0].numberData;
	int action = (int)arguments[1].numberData;

	if(action == 1) {
		vector<binds::Keybind> &binds = engine->gamepadToBind[button];
		for(auto &bind: binds) {
			// handle game objects
			tsl::robin_set<GameObject*> presses = engine->bindToGameObject[bind.bind];
			for(GameObject* object: presses) {
				object->onBind(bind.bind, binds::PRESS);
			}

			// handle TS callbacks
			tsl::robin_set<string> esPresses = engine->bindToTSCallback[bind.bind];
			for(auto &callback: esPresses) {
				esEntry arguments[1];
				arguments[0].type = ES_ENTRY_NUMBER;
				arguments[0].numberData = 1;
				esCallFunction(engine->eggscript, callback.c_str(), 1, arguments);
			}

			// handle TS object callbacks
			tsl::robin_set<pair<esObjectReferencePtr, string>> esObjectPresses = engine->bindToTSObjectCallback[bind.bind];
			for(auto &[object, callback]: esObjectPresses) {
				if(object->objectWrapper != nullptr) { // TODO delete from vector if object was deleted
					esEntry arguments[2];
					esCreateObjectAt(&arguments[0], object);
					esCreateNumberAt(&arguments[1], action);
					esCallMethod(engine->eggscript, object, callback.c_str(), 2, arguments);
				}
			}
		}
	}
	else if(action == 0) {
		vector<binds::Keybind> &binds = engine->gamepadToBind[button];
		for(auto &bind: binds) {
			// handle game objects
			tsl::robin_set<GameObject*> presses = engine->bindToGameObject[bind.bind];
			for(GameObject* object: presses) {
				object->onBind(bind.bind, binds::RELEASE);
			}

			// handle TS callbacks
			tsl::robin_set<string> esPresses = engine->bindToTSCallback[bind.bind];
			for(auto &callback: esPresses) {
				esEntry arguments[1];
				arguments[0].type = ES_ENTRY_NUMBER;
				arguments[0].numberData = 0;
				esCallFunction(engine->eggscript, callback.c_str(), 1, arguments);
			}

			// handle TS object callbacks
			tsl::robin_set<pair<esObjectReferencePtr, string>> esObjectPresses = engine->bindToTSObjectCallback[bind.bind];
			for(auto &[object, callback]: esObjectPresses) {
				if(object->objectWrapper != nullptr) { // TODO delete from vector if object was deleted
					esEntry arguments[2];
					esCreateObjectAt(&arguments[0], object);
					esCreateNumberAt(&arguments[1], action);
					esCallMethod(engine->eggscript, object, callback.c_str(), 2, arguments);
				}
			}
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
	esEntry arguments1[2];
	arguments1[0].type = ES_ENTRY_NUMBER;
	arguments1[0].numberData = binds::MOUSE_AXIS_X;
	arguments1[1].type = ES_ENTRY_NUMBER;
	arguments1[1].numberData = x;
	esCallFunction(engine->eggscript, "onAxisMove", 2, arguments1);

	esEntry arguments2[2];
	arguments2[0].type = ES_ENTRY_NUMBER;
	arguments2[0].numberData = binds::MOUSE_AXIS_Y;
	arguments2[1].type = ES_ENTRY_NUMBER;
	arguments2[1].numberData = y;
	esCallFunction(engine->eggscript, "onAxisMove", 2, arguments2);
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

void onGamepadButton(binds::GamepadButtons bind, bool pressed) {
	esEntry arguments[2];
	arguments[0].type = ES_ENTRY_NUMBER;
	arguments[0].numberData = bind;
	arguments[1].type = ES_ENTRY_NUMBER;
	arguments[1].numberData = pressed;
	esCallFunction(engine->eggscript, "onGamepadButton", 2, arguments);
}
