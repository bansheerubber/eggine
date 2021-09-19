#include "debug.h"

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
