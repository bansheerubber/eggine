#include "connection.h"

#ifndef _WIN32
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "../engine/console.h"
#include "../engine/engine.h"
#include "packet.h"
#include "../util/random.h"

network::Connection::Connection(int64_t _socket, sockaddr_in6 address) {
	this->_socket = _socket;
	this->tcpAddress = address;
	this->ip = IPAddress(address);
	this->secret = randomLong();

	this->lastSequenceReceived = randomInt();
	this->lastSequenceSent = randomInt();
	this->lastHighestAckReceived = this->lastSequenceSent;
}

network::Connection::~Connection() {
	if(this->_socket > 0) {
		::close(this->_socket);
	}

	console::print("dropped connection %s\n", this->ip.toString().c_str());

	engine->network.removeConnection(this);
}

void network::Connection::receiveTCP() {
	this->receiveStream->allocate(EGGINE_PACKET_SIZE);
	this->receiveStream->flush();
	
	#ifdef _WIN32
	int length = ::recv((SOCKET)this->_socket, &this->receiveStream->buffer[0], EGGINE_PACKET_SIZE, 0);
	if(length < 0) {
		return;
	}
	else if(length == 0) {
		return;
	}
	#else
	int length = ::recv(this->_socket, &this->receiveStream->buffer[0], EGGINE_PACKET_SIZE, 0);
	if(length < 0) {
		if(errno == EAGAIN || errno == EWOULDBLOCK) {
			return;
		}
		delete this;
		return;
	}
	else if(length == 0) {
		delete this;
		return;
	}
	#endif

	console::print("got a communication\n");

	this->receiveStream->buffer.head = length;

	if(!this->isInitialized()) { // this should be a checksum
		switch(this->handshake) {
			case WAIT_FOR_CHECKSUM: {
				if(length != 34) { // drop the packet if it isn't a checksum
					delete this;
					return;
				}

				std::string checksum = this->receiveStream->readString();
				if(checksum == engine->network.getChecksum()) { // send the secret if we got the right checksum
					Stream stream(WRITE);
					stream.writeNumber<uint64_t>(this->secret);
					stream.writeNumber<unsigned int>(this->lastSequenceReceived);
					stream.writeNumber<unsigned int>(this->lastSequenceSent);
					stream.writeNumber<char>(1);
					this->sendTCP(stream.size(), stream.start());
					this->handshake = WAIT_FOR_SECRET;

					console::print("checksum pass\n");
				}
				else {
					// TODO disconnect the client
					console::error("wrong checksum, bye\n");
					const char* error = "E:Wrong checksum";
					this->sendTCP(16, error);
					delete this;
					return;
				}
				break;
			}

			case WAIT_FOR_SECRET: { // we expect this connection to be silent if we're waiting for the secret
				delete this;
				return;
			}
		}
	}
}

void network::Connection::receiveUDP(Stream &stream) {
	if(!this->isInitialized() || this->handshake != PACKET_READY) { // something really wrong, close the connection
		delete this;
		return;
	}
	
	Stream* oldStream = this->receiveStream; // TODO fix this potential nightmare
	this->receiveStream = &stream;
	try {
		this->readPacket();
	}
	catch(StreamOverReadException &e) {
		console::error("%s\n", e.what());
	}
	this->receiveStream = oldStream;
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

void network::Connection::sendTCP(uint64_t size, const char* buffer) {
	#ifdef _WIN32
	::send((SOCKET)this->_socket, buffer, size, 0);
	#else
	::send(this->_socket, buffer, size, 0);
	#endif
}

void network::Connection::sendUDP(uint64_t size, const char* buffer) {
	#ifdef _WIN32
	::sendto((SOCKET)engine->network.getUDPSocket(), buffer, size, 0, (sockaddr*)&this->udpAddress, sizeof(this->udpAddress));
	#else
	::sendto(engine->network.getUDPSocket(), buffer, size, 0, (sockaddr*)&this->udpAddress, sizeof(this->udpAddress));
	#endif
}

void network::Connection::requestSecret()  {
	Stream stream(WRITE);
	stream.writeNumber<char>(1);
	this->sendTCP(stream.size(), stream.start());
}

void network::Connection::initializeUDP(sockaddr_in6 address) {
	this->handshake = PACKET_READY;
	this->udpAddress = address;
	
	Stream stream(WRITE);
	stream.writeNumber<char>(2);
	this->sendTCP(stream.size(), stream.start());
	this->initialized = true;

	console::print("initialized udp connection for %s\n", this->ip.toString().c_str());
}

void network::Connection::handlePacket() {
	
}
