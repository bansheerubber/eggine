#include "remoteObject.h"

#include <stdio.h>

#include "../engine/engine.h"
#include "../engine/console.h"

network::RemoteObject::RemoteObject() {
	engine->network.addRemoteObject(this);
}

network::RemoteObject::~RemoteObject() {
	engine->network.removeRemoteObject(this);
}

void network::RemoteObject::writeUpdateMask(unsigned int position) {
	if(this->updateMask == nullptr) {
		this->allocateMask();
	}
	
	unsigned int charIndex = position / 8;
	if(charIndex >= this->updateMaskSize) {
		return;
	}

	unsigned int maskIndex = position % 8;
	this->updateMask[charIndex] |= 1 << maskIndex;

	this->update = true;
}

void network::RemoteObject::writeUpdateMask(std::string property) {
	this->writeUpdateMask(this->propertyToMaskPosition(property));
}

bool network::RemoteObject::readUpdateMask(unsigned int position) {
	if(this->updateMask == nullptr) {
		this->allocateMask();
	}
	
	unsigned int charIndex = position / 8;
	if(charIndex >= this->updateMaskSize) {
		return false;
	}

	unsigned int maskIndex = position % 8;
	return this->updateMask[charIndex] & (1 << maskIndex);
}

void network::RemoteObject::zeroUpdateMask() {
	if(this->updateMask == nullptr) {
		this->allocateMask();
	}
	
	for(unsigned int i = 0; i < this->updateMaskSize; i++) {
		this->updateMask[i] = 0;
	}

	this->update = false;
}

void network::RemoteObject::printUpdateMask() {
	if(this->updateMask == nullptr) {
		this->allocateMask();
	}
	
	for(unsigned char i = 0; i < this->updateMaskSize * sizeof(char) * 8; i++) {
		console::print("%d", this->readUpdateMask(i));
	}
	console::print("\n");
}

bool network::RemoteObject::hasUpdate() {
	return this->update;
}

network::remote_object_id network::RemoteObject::getRemoteId() {
	return this->remoteId;
}

network::remote_class_id network::RemoteObject::getRemoteClassId() {
	return -1;
}

void network::RemoteObject::pack(Packet* packet) {
	
}

void network::RemoteObject::unpack(Stream &stream) {
	
}

void network::RemoteObject::allocateMask() {
	
}

unsigned int network::RemoteObject::propertyToMaskPosition(std::string property) {
	return 0;
}
