#include "client.h"

#include <algorithm>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <sys/types.h>

#include "connection.h"
#include "../engine/engine.h"
#include "packet.h"
#include "../basic/remoteObject.h"

// ##1 remote_object__headers

using namespace std;

network::Client::Client() {
	this->lastSequenceSent = 100000; // start us off at a high sequence number so i can debug easier
}

network::Client::~Client() {

}

bool network::Client::isActive() {
	return this->tcpSocket != -1;
}

void network::Client::open() {
	sockaddr_in6 serverAddress;

	serverAddress.sin6_family = AF_INET6;
	serverAddress.sin6_addr = in6addr_any;
	serverAddress.sin6_port = htons(28000);
	
	// ininitalize tcp socket
	{
		this->tcpSocket = socket(AF_INET6, SOCK_STREAM, 0);
		if(this->tcpSocket < 0) {
			printf("could not instantiate tcp socket\n");
			return;
		}

		if(connect(this->tcpSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
			printf("could not connect tcp socket\n");
			this->tcpSocket = -1;
			return;
		}

		fcntl(this->tcpSocket, F_SETFL, O_NONBLOCK);
	}

	// initialize udp socket
	{
		this->udpSocket = socket(AF_INET6, SOCK_DGRAM, 0);
		if(this->udpSocket < 0) {
			printf("could not instantiate udp socket\n");
			return;
		}

		if(connect(this->udpSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
			printf("could not udp socket\n");
			this->udpSocket = -1;
			return;
		}

		fcntl(this->udpSocket, F_SETFL, O_NONBLOCK);
	}
}

void network::Client::close() {

}

void network::Client::tick() {
	this->receiveTCP();
	this->receiveUDP();
}

void network::Client::receiveTCP() {
	this->receiveStream->allocate(EGGINE_PACKET_SIZE);
	this->receiveStream->flush();
	int length = ::recv(this->tcpSocket, &this->receiveStream->buffer[0], EGGINE_PACKET_SIZE, 0);
	if(length < 0) {
		if(errno == EWOULDBLOCK) {
			return;
		}
	}
	else if(length == 0) {
		return;
	}

	this->receiveStream->buffer.head = length;

	// if our connection isn't initialized, we should receive a secret as our first message
	if(!this->initialized) {
		if(this->secret == 0) {
			this->secret = this->receiveStream->readNumber<unsigned long>();
		}

		printf("got secret %ld\n", this->secret);

		// simple protocol for negotiating UDP connection
		while(this->receiveStream->canRead(1)) {
			switch(this->receiveStream->readNumber<char>()) {
				case 1: { // transmit secret via UDP
					Stream stream(WRITE);
					stream.writeNumber<unsigned long>(this->secret);
					::send(this->udpSocket, stream.start(), 8, 0);
					break;
				}

				case 2: { // we're initialized
					this->initialized = true;
					break;
				}
			}
		}
	}
}

void network::Client::receiveUDP() {
	this->receiveStream->allocate(EGGINE_PACKET_SIZE);
	this->receiveStream->flush();
	int length = ::recv(this->udpSocket, &this->receiveStream->buffer[0], EGGINE_PACKET_SIZE, 0);
	if(length < 0) {
		if(errno == EWOULDBLOCK) {
			return;
		}
	}
	else if(length == 0) {
		return;
	}

	this->receiveStream->buffer.head = length;

	// randomly drop packets
	// double result = (double)rand() / (double)RAND_MAX;
	// if(result < 0.5) {
	// 	return;
	// }

	// handle packet
	this->readPacket();

	// send a packet of our very own
	Packet* packet = new Packet();
	packet->setType(DROPPABLE_PACKET);
	this->sendPacket(packet);
}

void network::Client::handlePacket() {
	try {
		while(this->receiveStream->canRead(2)) { // read the size of the next chunk
			unsigned short size = this->receiveStream->readNumber<unsigned short>();
			StreamType type = (StreamType)this->receiveStream->readNumber<char>();
			switch(type) {
				case REMOTE_OBJECT_CREATE: {
					remote_class_id remoteClassId = this->receiveStream->readNumber<remote_class_id>();
					remote_object_id remoteObjectId = this->receiveStream->readNumber<remote_object_id>();

					RemoteObject* object = this->instantiateRemoteObject(remoteObjectId, remoteClassId);
					if(object != nullptr) {
						object->unpack(*this->receiveStream);
					}
					else {
						throw 0;
					}

					break;
				}
				
				case REMOTE_OBJECT_UPDATE: {
					remote_class_id remoteClassId = this->receiveStream->readNumber<remote_class_id>();
					remote_object_id remoteObjectId = this->receiveStream->readNumber<remote_object_id>();

					RemoteObject* object = engine->network.getRemoteObject(remoteObjectId);
					if(object != nullptr) {
						object->unpack(*this->receiveStream);
					}
					else {
						throw 0;
					}

					break;
				}
			}
		}
	}
	catch(...) {
		printf("epic fail\n");
	}
}

void network::Client::sendPacket(Packet* packet) {
	this->lastSequenceSent++;
	packet->setHeader(this->lastSequenceSent, this->lastSequenceReceived, this->ackMask);
	this->send(packet->stream.size(), packet->stream.start());

	PacketHandler::sendPacket(packet);
}

void network::Client::send(size_t size, const char* buffer) {
	::send(this->tcpSocket, buffer, size, 0);
}

void network::Client::addRemoteObject(RemoteObject* remoteObject) {
	this->remoteObjects.push_back(remoteObject);
}

void network::Client::removeRemoteObject(RemoteObject* remoteObject) {
	this->remoteObjects.erase(find(this->remoteObjects.begin(), this->remoteObjects.end(), remoteObject));
}

network::RemoteObject* network::Client::instantiateRemoteObject(remote_object_id remoteId, remote_class_id remoteClassId) {
	RemoteObject* object = nullptr;

	if(engine->network.getRemoteObject(remoteId)) {
		return engine->network.getRemoteObject(remoteId);
	}

	switch(remoteClassId) {
		// ##1 remote_object_instantiation
	}
	
	if(object != nullptr) {
		object->remoteId = remoteId;
	}

	return object;
}
