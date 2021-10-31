#include "keybind.h"
#include "engine.h"

void Engine::registerTSKeybindCallback(string bind, string key, string callback) {
	if(this->keyToScancode.find(key) != this->keyToScancode.end()) {
		this->addKeybind(this->keyToScancode[key], {
			bind: bind,
		});

		this->bindToTSCallback[bind].push_back(callback);
	}
	else if(this->gamepadToEnum.find(key) != this->gamepadToEnum.end()) {
		this->addGamepadBind(this->gamepadToEnum[key], {
			bind: bind,
		});

		this->bindToTSCallback[bind].push_back(callback);
	}
	else if(this->axesToEnum.find(key) != this->axesToEnum.end()) {
		this->addAxis(this->axesToEnum[key], {
			bind: bind,
		});

		this->bindToTSCallback[bind].push_back(callback);
	}
	else if(this->mouseToEnum.find(key) != this->mouseToEnum.end()) {
		this->addMousebind(this->mouseToEnum[key], {
			bind: bind,
		});

		this->bindToTSCallback[bind].push_back(callback);
	}
}

void Engine::registerTSKeybindObjectCallback(esObjectReferencePtr object, string bind, string key, string callback) {
	if(this->keyToScancode.find(key) != this->keyToScancode.end()) {
		this->addKeybind(this->keyToScancode[key], {
			bind: bind,
		});

		this->bindToTSObjectCallback[bind].emplace_back(esCloneObjectReference(object), callback);
	}
	else if(this->gamepadToEnum.find(key) != this->gamepadToEnum.end()) {
		this->addGamepadBind(this->gamepadToEnum[key], {
			bind: bind,
		});

		this->bindToTSObjectCallback[bind].emplace_back(esCloneObjectReference(object), callback);
	}
	else if(this->axesToEnum.find(key) != this->axesToEnum.end()) {
		this->addAxis(this->axesToEnum[key], {
			bind: bind,
		});

		this->bindToTSObjectCallback[bind].emplace_back(esCloneObjectReference(object), callback);
	}
	else if(this->mouseToEnum.find(key) != this->mouseToEnum.end()) {
		this->addMousebind(this->mouseToEnum[key], {
			bind: bind,
		});

		this->bindToTSObjectCallback[bind].emplace_back(esCloneObjectReference(object), callback);
	}
}

void Engine::registerBind(string command, GameObject* gameObject) {
	this->bindToGameObject[command].push_back(gameObject);
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
