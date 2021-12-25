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

// ##1 remote_object_headers

using namespace std;

network::Client::Client() {
	this->lastSequenceSent = 100000; // start us off at a high sequence number so i can debug easier
}

network::Client::~Client() {

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

		// send network checksum right away
		Stream stream;
		stream.writeString(engine->network.getChecksum());
		::send(this->tcpSocket, stream.start(), stream.size(), 0);

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

void network::Client::receive() {
	this->receiveTCP();
	this->receiveUDP();
}

void network::Client::receiveTCP() {
	if(this->initialized) {
		return;
	}
	
	this->receiveStream->allocate(EGGINE_PACKET_SIZE);
	this->receiveStream->flush();
	int length = ::recv(this->tcpSocket, &this->receiveStream->buffer[0], EGGINE_PACKET_SIZE, 0);
	if(length < 0) {
		if(errno == EWOULDBLOCK) {
			return;
		}
		return;
	}
	else if(length == 0) {
		return;
	}

	this->receiveStream->buffer.head = length;

	// if our connection isn't initialized, we should receive a secret as our first message
	if(!this->initialized) {
		if(!this->hasSecret) {
			this->secret = this->receiveStream->readNumber<unsigned long>();
			this->hasSecret = true;

			this->lastSequenceSent = this->receiveStream->readNumber<unsigned int>();
			this->lastSequenceReceived = this->receiveStream->readNumber<unsigned int>();
			this->lastHighestAckReceived = this->lastSequenceSent;
		}

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
		return;
	}
	else if(length == 0) {
		return;
	}

	this->receiveStream->buffer.head = length;

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
						throw RemoteObjectInstantiateException(remoteObjectId, remoteClassId);
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
						throw RemoteObjectLookupException(remoteObjectId);
					}

					break;
				}
			}
		}
	}
	// if we couldn't instantiate a remote object, then terminate the connection. there may be a problem with:
	// 1. remote object constructor failure
	// 2. remote class id not being matched to a remote object constructor
	// and i think either of those is a fatal error that we can't recover from because:
	// 1. if we can't do `new ...` without a constructor failure, then this remote object shouldn't have been
	//    replicated in the first place. server is going crazy
	// 2. if we can't match a remote class id to a remote object constructor, then there is something wrong
	//    with the packet that could be a). a data corruption problem which means the server has gone haywire or
	//    b). the client and server are somehow out of sync on which remote class id matches to which remote class
	catch(RemoteObjectInstantiateException &e) {
		printf("%s\n", e.what());
	}
	// if we couldn't look up a remote object, then just log it, drop the packet, and continue.
	// TODO is there a better behavior for this? i don't want to read the full packet, but dropping data is bad. maybe
	//      ask the server to create a rescue packet for us?
	catch(RemoteObjectLookupException &e) {
		printf("%s\n", e.what());
	}
	// if we over-read the stream, then terminate the connection. something has gone really bad
	catch(StreamOverReadException &e) {
		printf("%s\n", e.what());
	}
	// this is thrown when a setter doesn't like the data it given during the unpacking phase. this could be caused
	// by straight up bad data, or something as simple as out-of-bounds values. log the error and continue, since we
	// don't know which it could be. if its straight up bad data and the server has gone crazy, then there's many other
	// checks in place for us to discover that
	catch(RemoteObjectUnpackException &e) {
		printf("%s\n", e.what());
	}
	// if the remote object id supplied at the end of a update chunk does not match the id we read at the beginning, then
	// this is enough evidence to say that the server has gone crazy. terminate the connection
	catch(RemoteObjectIdMisMatchException &e) {
		printf("%s\n", e.what());
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
	::send(this->udpSocket, buffer, size, 0);
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
