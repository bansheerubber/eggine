#include "keybind.h"
#include "engine.h"

void Engine::registerTSKeybindCallback(string bind, string key, string callback) {
	if(this->keyToScancode.find(key) != this->keyToScancode.end()) {
		this->addKeybind(this->keyToScancode[key], {
			bind: bind,
		});

		this->bindToTSCallback[bind].push_back(callback);
	}
}

void Engine::registerBindPress(string command, GameObject* gameObject) {
	this->bindPressToGameObject[command].push_back(gameObject);
}

void Engine::registerBindRelease(string command, GameObject* gameObject) {
	this->bindReleaseToGameObject[command].push_back(gameObject);
}

void Engine::registerBindAxis(string command, GameObject* gameObject) {
	this->bindAxisToGameObject[command].push_back(gameObject);
}

void Engine::addKeybind(int key, binds::Keybind keybind) {
	this->keyToKeybind[key].push_back(keybind);
}

void Engine::addAxis(binds::Axes axis, binds::Keybind keybind) {
	this->axisToKeybind[axis].push_back(keybind);
}

void Engine::addMousebind(int button, binds::Keybind keybind) {
	this->buttonToMousebind[button].push_back(keybind);
}

void Engine::addGamepadBind(binds::GamepadButtons bind, binds::Keybind keybind) {
	this->gamepadToBind[bind].push_back(keybind);
}
