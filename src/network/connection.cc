#include "connection.h"

#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "packet.h"

network::Connection::Connection(int _socket, sockaddr_in6 address) {
	this->_socket = _socket;
	this->address = address;
	this->ip = ConnectionIPAddress(address);
}

void network::Connection::sendPacket(Packet* packet) {
	this->lastSequenceSent++;
	packet->setHeader(this->lastSequenceSent, this->lastSequenceReceived, this->ackMask);
	this->send(packet->stream.size(), packet->stream.start());

	PacketHandler::sendPacket(packet);
}

void network::Connection::send(size_t size, const char* buffer) {
	::send(this->_socket, buffer, size, 0);
}

void network::Connection::recv() {
	this->receiveStream.allocate(EGGINE_PACKET_SIZE);
	this->receiveStream.flush();
	int length = ::recv(this->_socket, &this->receiveStream.buffer[0], EGGINE_PACKET_SIZE, 0);
	if(length < 0) {
		if(errno == EWOULDBLOCK) {
			return;
		}
	}
	else if(length == 0) {
		return;
	}

	this->receiveStream.buffer.head = length;

	// handle packet
	this->readPacket();
}
