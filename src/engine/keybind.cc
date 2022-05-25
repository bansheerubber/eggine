#include "keybind.h"
#include "engine.h"

void Engine::registerTSKeybindCallback(std::string bind, std::string key, std::string callback) {
	if(this->keyToScancode.find(key) != this->keyToScancode.end()) {
		this->addKeybind(this->keyToScancode[key], {
			bind: bind,
		});

		this->bindToTSCallback[bind].emplace(callback);
	}
	else if(this->gamepadToEnum.find(key) != this->gamepadToEnum.end()) {
		this->addGamepadBind(this->gamepadToEnum[key], {
			bind: bind,
		});

		this->bindToTSCallback[bind].emplace(callback);
	}
	else if(this->axesToEnum.find(key) != this->axesToEnum.end()) {
		this->addAxis(this->axesToEnum[key], {
			bind: bind,
		});

		this->bindToTSCallback[bind].emplace(callback);
	}
	else if(this->mouseToEnum.find(key) != this->mouseToEnum.end()) {
		this->addMousebind(this->mouseToEnum[key], {
			bind: bind,
		});

		this->bindToTSCallback[bind].emplace(callback);
	}
}

void Engine::registerTSKeybindObjectCallback(esObjectReferencePtr object, std::string bind, std::string key, std::string callback) {
	if(this->keyToScancode.find(key) != this->keyToScancode.end()) {
		this->addKeybind(this->keyToScancode[key], {
			bind: bind,
		});

		if(this->bindToTSObjectCallback[bind].find(std::pair<esObjectReferencePtr, std::string>(object, callback)) == this->bindToTSObjectCallback[bind].end()) {
			this->bindToTSObjectCallback[bind].emplace(esCloneObjectReference(object), callback);
		}
	}
	else if(this->gamepadToEnum.find(key) != this->gamepadToEnum.end()) {
		this->addGamepadBind(this->gamepadToEnum[key], {
			bind: bind,
		});

		if(this->bindToTSObjectCallback[bind].find(std::pair<esObjectReferencePtr, std::string>(object, callback)) == this->bindToTSObjectCallback[bind].end()) {
			this->bindToTSObjectCallback[bind].emplace(esCloneObjectReference(object), callback);
		}
	}
	else if(this->axesToEnum.find(key) != this->axesToEnum.end()) {
		this->addAxis(this->axesToEnum[key], {
			bind: bind,
		});

		if(this->bindToTSObjectCallback[bind].find(std::pair<esObjectReferencePtr, std::string>(object, callback)) == this->bindToTSObjectCallback[bind].end()) {
			this->bindToTSObjectCallback[bind].emplace(esCloneObjectReference(object), callback);
		}
	}
	else if(this->mouseToEnum.find(key) != this->mouseToEnum.end()) {
		this->addMousebind(this->mouseToEnum[key], {
			bind: bind,
		});

		if(this->bindToTSObjectCallback[bind].find(std::pair<esObjectReferencePtr, std::string>(object, callback)) == this->bindToTSObjectCallback[bind].end()) {
			this->bindToTSObjectCallback[bind].emplace(esCloneObjectReference(object), callback);
		}
	}
}

void Engine::registerBind(std::string command, GameObject* gameObject) {
	this->bindToGameObject[command].emplace(gameObject);
}

void Engine::registerBindAxis(std::string command, GameObject* gameObject) {
	this->bindAxisToGameObject[command].emplace(gameObject);
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
