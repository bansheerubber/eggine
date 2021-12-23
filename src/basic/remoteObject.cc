#include "remoteObject.h"

#include <stdio.h>

#include "../engine/engine.h"

network::RemoteObject::RemoteObject() {
	engine->network.addRemoteObject(this);
}

network::RemoteObject::~RemoteObject() {
	engine->network.removeRemoteObject(this);
}

void network::RemoteObject::allocateMask() {
	
}

void network::RemoteObject::pack(Packet* packet) {
	
}

bool network::RemoteObject::hasUpdate() {
	return this->update;
}

unsigned int network::RemoteObject::propertyToMaskPosition(string property) {
	return 0;
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

void network::RemoteObject::writeUpdateMask(string property) {
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

void network::RemoteObject::printUpdateMask() {
	if(this->updateMask == nullptr) {
		this->allocateMask();
	}
	
	for(unsigned int i = 0; i < this->updateMaskSize * sizeof(char) * 8; i++) {
		printf("%d", this->readUpdateMask(i));
	}
	printf("\n");
}

unsigned long network::RemoteObject::getRemoteId() {
	return this->remoteId;
}

unsigned short network::RemoteObject::getRemoteClassId() {
	return -1;
}