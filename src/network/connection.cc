#include "connection.h"

#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "../engine/engine.h"
#include "packet.h"

network::Connection::Connection(int _socket, sockaddr_in6 address) {
	this->_socket = _socket;
	this->tcpAddress = address;
	this->ip = ConnectionIPAddress(address);

	Stream stream(WRITE);
	stream.writeNumber<unsigned long>(this->secret);
	stream.writeNumber<char>(1);
	this->sendTCP(stream.size(), stream.start());
}

void network::Connection::sendPacket(Packet* packet) {
	this->lastSequenceSent++;
	packet->setHeader(this->lastSequenceSent, this->lastSequenceReceived, this->ackMask);
	this->sendUDP(packet->stream.size(), packet->stream.start());

	PacketHandler::sendPacket(packet);
}

bool network::Connection::isInitialized() {
	return this->initialized;
}

void network::Connection::sendTCP(size_t size, const char* buffer) {
	::send(this->_socket, buffer, size, 0);
}

void network::Connection::sendUDP(size_t size, const char* buffer) {
	::sendto(engine->network.getUDPSocket(), buffer, size, 0, (sockaddr*)&this->udpAddress, sizeof(this->udpAddress));
}

void network::Connection::requestSecret()  {
	Stream stream(WRITE);
	stream.writeNumber<char>(1);
	this->sendTCP(stream.size(), stream.start());
}

void network::Connection::initializeUDP(sockaddr_in6 address) {
	this->udpAddress = address;
	
	Stream stream(WRITE);
	stream.writeNumber<char>(2);
	this->sendTCP(stream.size(), stream.start());
	this->initialized = true;

	printf("initialized udp connection for %s\n", this->ip.toString().c_str());
}

void network::Connection::receiveTCP() {
	this->receiveStream->allocate(EGGINE_PACKET_SIZE);
	this->receiveStream->flush();
	int length = ::recv(this->_socket, &this->receiveStream->buffer[0], EGGINE_PACKET_SIZE, 0);
	if(length < 0) {
		if(errno == EWOULDBLOCK) {
			return;
		}
	}
	else if(length == 0) {
		return;
	}

	this->receiveStream->buffer.head = length;

	// handle packet
	this->readPacket();
}

void network::Connection::receiveUDP(Stream &stream) {
	Stream* oldStream = this->receiveStream; // TODO fix this potential nightmare
	this->readPacket();
	this->receiveStream = oldStream;
}

void network::Connection::handlePacket() {
	
}
