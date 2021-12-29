#include "stream.h"

#ifndef _WIN32
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#endif

#include <algorithm>
#include <thread>

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

void network::Stream::startChunk() {
	this->chunkHead = this->buffer.head;
	this->writeNumber<unsigned short>(0);
}

void network::Stream::commitChunk() {
	size_t oldHead = this->buffer.head;
	unsigned short size = this->buffer.head - this->chunkHead;
	this->buffer.head = this->chunkHead;
	this->writeNumber<unsigned short>(size);
	this->buffer.head = oldHead;
}

void network::Stream::startWriteRemoteObject(RemoteObject* object) {
	this->writeNumber(object->getRemoteClassId());
	this->writeNumber(object->getRemoteId());
}

void network::Stream::finishWriteRemoteObject(RemoteObject* object) {
	this->writeNumber(object->getRemoteId());
	this->commitChunk();
}

void network::Stream::finishReadRemoteObject(class RemoteObject* object) {
	remote_object_id id = this->readNumber<remote_object_id>();
	if(id != object->getRemoteId()) {
		throw RemoteObjectIdMisMatchException(object->getRemoteId(), id);
	}
}

void network::Stream::writeUpdateMask(unsigned int size, const unsigned char* mask) {
	this->writeNumber((unsigned char)size);
	for(unsigned int i = 0; i < size; i++) {
		if(this->flags & NO_MASK_CHECKING) {
			this->writeNumber<unsigned char>(0xFF);
		}
		else {
			this->writeNumber<unsigned char>(mask[i]);
		}
	}
}

const network::UpdateMask network::Stream::readUpdateMask() {
	UpdateMask mask;
	unsigned char size = this->readNumber<unsigned char>();
	if(size > EGGINE_NETWORK_MAX_UPDATE_MASK_SIZE) {
		mask.size = 255;
		return mask;
	}
	mask.size = size;

	for(unsigned char i = 0; i < size; i++) {
		mask.mask[i] = this->readNumber<unsigned char>();
	}
	return mask;
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