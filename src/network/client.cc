#include "client.h"

#ifdef _WIN32
#include <Ws2tcpip.h>
#include <winsock2.h>
#else
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#endif

#include <algorithm>
#include <thread>


#include "connection.h"
#include "../engine/console.h"
#include "../engine/engine.h"
#include "packet.h"
#include "../basic/remoteObject.h"

// ##1 remote_object_headers

network::Client::Client() {
	this->lastSequenceSent = 100000; // start us off at a high sequence number so i can debug easier
}

network::Client::~Client() {

}

void network::Client::open(std::string ip, unsigned short port) {
	if(ip.length() > 40 || port < 500) {
		return;
	}

	std::string portString = std::to_string(port);
	
	#ifdef _WIN32
	// setup tcp socket
	{
		addrinfo* result = nullptr;
		addrinfo hints;

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET6;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		getaddrinfo(ip.c_str(), portString.c_str(), &hints, &result);
		
		this->tcpSocket = socket(result->ai_family, SOCK_STREAM, IPPROTO_TCP);
		if((SOCKET)this->tcpSocket == INVALID_SOCKET) {
			console::error("could not instantiate tcp socket: %d\n", WSAGetLastError());
			return;
		}

		if(connect((SOCKET)this->tcpSocket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
			console::error("could not connect tcp socket: %d\n", WSAGetLastError());
			this->tcpSocket = (uint64_t)INVALID_SOCKET;
			return;
		}

		// send network checksum right away
		Stream stream;
		stream.writeString(engine->network.getChecksum());
		::send((SOCKET)this->tcpSocket, stream.start(), (int)stream.size(), 0);

		u_long block = 1;
		ioctlsocket((SOCKET)this->tcpSocket, FIONBIO, &block);
	}

	// initialize udp socket
	{
		addrinfo* result = nullptr;
		addrinfo hints;

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET6;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;
		hints.ai_flags = AI_PASSIVE;

		getaddrinfo(ip.c_str(), portString.c_str(), &hints, &result);
		
		this->udpSocket = socket(result->ai_family, SOCK_DGRAM, IPPROTO_UDP);
		if((SOCKET)this->udpSocket == INVALID_SOCKET) {
			console::error("could not instantiate udp socket: %d\n", WSAGetLastError());
			return;
		}

		if(connect((SOCKET)this->udpSocket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
			console::error("could not udp socket: %d\n", WSAGetLastError());
			this->udpSocket = (uint64_t)INVALID_SOCKET;
			return;
		}

		u_long block = 1;
		ioctlsocket((SOCKET)this->udpSocket, FIONBIO, &block);

		this->ip = *(sockaddr_in6*)result->ai_addr;
	}
	#else
	// ininitalize tcp socket
	{
		addrinfo* result = nullptr;
		addrinfo hints;

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET6;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		getaddrinfo(ip.c_str(), portString.c_str(), &hints, &result);
		
		this->tcpSocket = socket(AF_INET6, SOCK_STREAM, 0);
		if(this->tcpSocket < 0) {
			console::error("could not instantiate tcp socket\n");
			return;
		}

		if(connect(this->tcpSocket, result->ai_addr, result->ai_addrlen) < 0) {
			console::error("could not connect tcp socket\n");
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
		addrinfo* result = nullptr;
		addrinfo hints;

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET6;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;
		hints.ai_flags = AI_PASSIVE;

		getaddrinfo(ip.c_str(), portString.c_str(), &hints, &result);
		
		this->udpSocket = socket(AF_INET6, SOCK_DGRAM, 0);
		if(this->udpSocket < 0) {
			console::error("could not instantiate udp socket\n");
			return;
		}

		if(connect(this->udpSocket, result->ai_addr, result->ai_addrlen) < 0) {
			console::error("could not udp socket\n");
			this->udpSocket = -1;
			return;
		}

		fcntl(this->udpSocket, F_SETFL, O_NONBLOCK);

		this->ip = *((sockaddr_in6*)result->ai_addr);
	}
	#endif
}

void network::Client::close() {

}

void network::Client::receive() {
	this->receiveTCP();
	this->receiveUDP();
}

void network::Client::receiveTCP() {
	#ifdef _WIN32
	if(this->initialized) {
		return;
	}
	
	this->receiveStream->allocate(EGGINE_PACKET_SIZE);
	this->receiveStream->flush();
	int length = ::recv((SOCKET)this->tcpSocket, &this->receiveStream->buffer[0], EGGINE_PACKET_SIZE, 0);
	if(length < 0) {
		if(WSAGetLastError() == WSAEWOULDBLOCK) {
			return;
		}
		return;
	}
	else if(length == 0) {
		return;
	}

	this->receiveStream->buffer.head = length;
	#else
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
	#endif

	// if our connection isn't initialized, we should receive a secret as our first message
	if(!this->initialized) {
		if(!this->hasSecret) {
			this->secret = this->receiveStream->readNumber<uint64_t>();
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
					stream.writeNumber<uint64_t>(this->secret);
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
	#ifdef _WIN32
	int length = ::recv((SOCKET)this->udpSocket, &this->receiveStream->buffer[0], EGGINE_PACKET_SIZE, 0);
	if(length < 0) {
		if(WSAGetLastError() == WSAEWOULDBLOCK) {
			return;
		}
		return;
	}
	else if(length == 0) {
		return;
	}

	this->receiveStream->buffer.head = length;
	#else
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
	#endif

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
			this->receiveStream->readNumber<unsigned short>(); // TODO use size to do bug checking
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
					this->receiveStream->readNumber<remote_class_id>(); // remote class id
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
		console::error("%s\n", e.what());
	}
	// if we couldn't look up a remote object, then just log it, drop the packet, and continue.
	// TODO is there a better behavior for this? i don't want to read the full packet, but dropping data is bad. maybe
	//      ask the server to create a rescue packet for us?
	catch(RemoteObjectLookupException &e) {
		console::error("%s\n", e.what());
	}
	// if we over-read the stream, then terminate the connection. something has gone really bad
	catch(StreamOverReadException &e) {
		console::error("%s\n", e.what());
	}
	// this is thrown when a setter doesn't like the data it given during the unpacking phase. this could be caused
	// by straight up bad data, or something as simple as out-of-bounds values. log the error and continue, since we
	// don't know which it could be. if its straight up bad data and the server has gone crazy, then there's many other
	// checks in place for us to discover that
	catch(RemoteObjectUnpackException &e) {
		console::error("%s\n", e.what());
	}
	// if the remote object id supplied at the end of a update chunk does not match the id we read at the beginning, then
	// this is enough evidence to say that the server has gone crazy. terminate the connection
	catch(RemoteObjectIdMisMatchException &e) {
		console::error("%s\n", e.what());
	}
	catch(...) {
		console::error("epic fail\n");
	}
}

void network::Client::sendPacket(Packet* packet) {
	this->lastSequenceSent++;
	packet->setHeader(this->lastSequenceSent, this->lastSequenceReceived, this->ackMask);
	this->send(packet->stream.size(), packet->stream.start());

	PacketHandler::sendPacket(packet);
}

const network::IPAddress network::Client::getIPAddress() {
	return this->ip;
}

void network::Client::send(uint64_t size, const char* buffer) {
	#ifdef _WIN32
	::send((SOCKET)this->udpSocket, buffer, size, 0);
	#else
	::send(this->udpSocket, buffer, size, 0);
	#endif
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
