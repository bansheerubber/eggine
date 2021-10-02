#include "debug.h"

#include "engine.h"
#include "../util/cloneString.h"

void glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam) {
	if(id == 131185) {
		return;
	}
	
	engine->debug.glDebugMessages.push_back({
		source: source,
		type: type,
		id: id,
		severity: severity,
		length: length,
		message: cloneString(message),
		parameter: userParam,
	});
}

void Debug::addInfoMessage(string message) {
	this->infoMessages.push_back(message);
}

string Debug::getInfoText() {
	string output;
	for(string &message: this->infoMessages) {
		output += message + '\n';
	}
	return output;
}

void Debug::clearInfoMessages() {
	this->infoMessages.clear();
}

void Debug::flushGLDebugMessages() {
	for(size_t i = 0; i < this->glDebugMessages.size(); i++) {
		// only print groups if they actually captured some sort of message
		if(
			this->glDebugMessages[i].id == 1
			&& i + 1 < this->glDebugMessages.size()
			&& this->glDebugMessages[i + 1].id != 1
		) {
			this->glDebugMessages[i].print();
		}
		else if(this->glDebugMessages[i].id != 1) {
			this->glDebugMessages[i].print();
		}
		else {
			i++;
		}
	}

	this->glDebugMessages.clear();
}
