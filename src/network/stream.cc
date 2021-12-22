#include "stream.h"

#include <algorithm>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <sys/types.h>

#include "client.h"
#include "connection.h"
#include "../basic/remoteObject.h"

network::Stream::Stream(unsigned int flags) {
	this->flags = flags;
	this->flush();
}

void network::Stream::setFlags(unsigned int flags) {
	this->flags = flags;
}

void network::Stream::allocate(size_t size) {
	this->buffer.allocate(size);
}

void network::Stream::startWriteRemoteObject(RemoteObject* object) {
	this->writeNumber((char)REMOTE_OBJECT_UPDATE);
	this->writeNumber(object->getRemoteClassId());
	this->writeNumber(object->getRemoteId());
}

void network::Stream::finishWriteRemoteObject(RemoteObject* object) {
	this->writeNumber(object->getRemoteId());
}

void network::Stream::writeMask(unsigned int size, const char* mask) {
	this->writeNumber((char)size);
	for(unsigned int i = 0; i < size; i++) {
		this->buffer[this->buffer.head] = (this->flags & NO_MASK_CHECKING) || mask[i];
		this->buffer.pushed();
	}
}

bool network::Stream::queryMask(RemoteObject* object, unsigned int position) {
	return (this->flags & NO_MASK_CHECKING) || object->readUpdateMask(position);
}

void network::Stream::flush() {
	this->buffer.head = 0;
	this->readBufferPointer = 0;
}

size_t network::Stream::size() {
	return this->buffer.head;
}

const char* network::Stream::start() {
	return &this->buffer[0];
}