#include "client.h"

#include <algorithm>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <sys/types.h>

#include "connection.h"
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
			printf("could not open socket\n");
			return;
		}

		if(connect(this->tcpSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
			printf("could not connect socket\n");
			this->tcpSocket = -1;
			return;
		}

		fcntl(this->tcpSocket, F_SETFL, O_NONBLOCK);
	}

	// initialize udp socket
	{
		this->udpSocket = socket(AF_INET6, SOCK_DGRAM, 0);
		if(this->udpSocket < 0) {
			printf("could not open socket\n");
			return;
		}

		if(connect(this->udpSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
			printf("could not connect socket\n");
			this->udpSocket = -1;
			return;
		}

		fcntl(this->udpSocket, F_SETFL, O_NONBLOCK);
	}
}

void network::Client::close() {

}

void network::Client::tick() {
	this->receiveStream.allocate(EGGINE_PACKET_SIZE);
	this->receiveStream.flush();
	int length = ::recv(this->tcpSocket, &this->receiveStream.buffer[0], EGGINE_PACKET_SIZE, 0);
	if(length < 0) {
		if(errno == EWOULDBLOCK) {
			return;
		}
	}
	else if(length == 0) {
		return;
	}

	this->receiveStream.buffer.head = length;

	// randomly drop packets
	double result = (double)rand() / (double)RAND_MAX;
	if(result < 0.5) {
		return;
	}

	// handle packet
	this->readPacket();

	// send a packet of our very own
	Packet* packet = new Packet();
	packet->setType(DROPPABLE_PACKET);
	this->sendPacket(packet);
}

void network::Client::sendPacket(Packet* packet) {
	this->lastSequenceSent++;
	packet->setHeader(this->lastSequenceSent, this->lastSequenceReceived, this->ackMask);
	this->send(packet->stream.size(), packet->stream.start());

	PacketHandler::sendPacket(packet);
}

void network::Client::send(size_t size, const char* buffer) {
	::send(this->tcpSocket, buffer, size, 0);

	char frog[16] = "hello there1234";
	::send(this->udpSocket, frog, 16, 0);
}

void network::Client::addRemoteObject(RemoteObject* remoteObject) {
	this->remoteObjects.push_back(remoteObject);
}

void network::Client::removeRemoteObject(RemoteObject* remoteObject) {
	this->remoteObjects.erase(find(this->remoteObjects.begin(), this->remoteObjects.end(), remoteObject));
}

void network::Client::instantiateRemoteObject(unsigned long remoteId, unsigned short remoteClassId) {
	RemoteObject* object = nullptr;
	switch(remoteClassId) {
		// ##1 remote_object_instantiation
	}
	
	if(object != nullptr) {
		object->remoteId = remoteId;
	}
}
