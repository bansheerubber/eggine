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

void Engine::registerBindHeld(string command, GameObject* gameObject) {
	this->bindHeldToGameObject[command].push_back(gameObject);
}

void Engine::addKeybind(int key, binds::Keybind keybind) {
	this->keyToKeybind[key].push_back(keybind);
}
