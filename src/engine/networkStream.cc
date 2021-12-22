#include "networkStream.h"

void network::Stream::flush() {
	this->buffer.head = 0;
}

void network::Stream::startWriteRemoteObject(unsigned int remoteId) {
	this->writeNumber((char)REMOTE_OBJECT_UPDATE);
	this->writeNumber(remoteId);
}

void network::Stream::finishWriteRemoteObject(unsigned int remoteId) {
	this->writeNumber(remoteId);
}

void network::Stream::writeMask(unsigned int size, const char* mask) {
	this->writeNumber((char)size);
	for(unsigned int i = 0; i < size; i++) {
		this->buffer[this->buffer.head] = mask[i];
		this->buffer.pushed();
	}
}