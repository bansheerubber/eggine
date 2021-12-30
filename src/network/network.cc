#include "network.h"

#ifdef _WIN32
#include <Ws2tcpip.h>
#include <winsock2.h>
#else
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#endif

#include <algorithm>
#include <iostream>
#include <thread>

#include "connection.h"
#include "../util/ipv6ToString.h"
#include "packet.h"
#include "../basic/remoteObject.h"

#include "../util/time.h"

network::Network::Network() {
}

network::Network::~Network() {
}

const network::IPAddress network::Network::getHostIPAddress() {
	return this->ip;
}

void network::Network::openServer() {
	this->mode = NETWORK_SERVER;
	
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

		getaddrinfo("localhost", "28000", &hints, &result);

		this->ip = *(sockaddr_in6*)result->ai_addr;

		this->tcpSocket = socket(result->ai_family, SOCK_STREAM, IPPROTO_TCP);
		if((SOCKET)this->tcpSocket == INVALID_SOCKET) {
			printf("could not instantiate tcp socket\n");
			return;
		}

		char enabled = 1;
		setsockopt((SOCKET)this->tcpSocket, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(int));

		if(::bind((SOCKET)this->tcpSocket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
			printf("could not bind tcp socket\n");
			return;
		}

		u_long block = 1;
		ioctlsocket((SOCKET)this->tcpSocket, FIONBIO, &block);

		listen(this->tcpSocket, 32);
	}

	// setup udp socket
	{
		addrinfo* result = nullptr;
		addrinfo hints;

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET6;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;
		hints.ai_flags = AI_PASSIVE;

		getaddrinfo("localhost", "28000", &hints, &result);

		this->udp.socket = socket(result->ai_family, SOCK_DGRAM, IPPROTO_UDP);
		if((SOCKET)this->udp.socket == INVALID_SOCKET) {
			printf("could not instantiate udp socket\n");
			return;
		}

		char enabled = 1;
		setsockopt((SOCKET)this->udp.socket, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(int));

		if(::bind((SOCKET)this->udp.socket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
			printf("could not bind udp socket\n");
			return;
		}

		u_long block = 1;
		ioctlsocket((SOCKET)this->udp.socket, FIONBIO, &block);

		listen((SOCKET)this->udp.socket, 32);
	}

	std::thread t(&Network::acceptServer, this);
	t.detach();
	#else
	sockaddr_in6 serverAddress;

	serverAddress.sin6_family = AF_INET6;
	serverAddress.sin6_addr = in6addr_any;
	serverAddress.sin6_port = htons(28000);

	// setup tcp socket
	{
		this->tcpSocket = socket(AF_INET6, SOCK_STREAM, 0);
		if(this->tcpSocket < 0) {
			printf("could not instantiate tcp socket\n");
			return;
		}

		int enabled = 1;
		setsockopt(this->tcpSocket, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(int));

		if(bind(this->tcpSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
			printf("could not bind tcp socket\n");
			return;
		}

		listen(this->tcpSocket, 32);

		fcntl(this->tcpSocket, F_SETFL, O_NONBLOCK);
	}

	// setup udp socket
	{
		this->udp.socket = socket(AF_INET6, SOCK_DGRAM, 0);
		if(this->udp.socket < 0) {
			printf("could not instantiate udp socket\n");
			return;
		}

		int enabled = 1;
		setsockopt(this->udp.socket, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(int));

		if(bind(this->udp.socket, (sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
			printf("could not bind tcp socket\n");
			return;
		}

		listen(this->udp.socket, 32);

		fcntl(this->udp.socket, F_SETFL, O_NONBLOCK);
	}

	this->ip = serverAddress;

	std::thread t(&Network::acceptServer, this);
	t.detach();
	#endif
}

void network::Network::closeServer() {

}

void network::Network::acceptServer() {
	while(true) {
		sockaddr_in6 clientAddress;
		socklen_t clientLength = sizeof(clientAddress);

		#ifdef _WIN32
		SOCKET clientSocket = ::accept((SOCKET)this->tcpSocket, (sockaddr*)&clientAddress, &clientLength);
		if(clientSocket == INVALID_SOCKET) {
			std::this_thread::sleep_for(std::chrono::milliseconds(16));
			continue;
		}

		u_long block = 1;
		ioctlsocket((SOCKET)clientSocket, FIONBIO, &block);
		#else
		int clientSocket = ::accept(this->tcpSocket, (sockaddr*)&clientAddress, &clientLength);
		if(clientSocket < 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(16));
			continue;
		}

		fcntl(clientSocket, F_SETFL, O_NONBLOCK);
		#endif

		Connection* connection = new Connection(clientSocket, clientAddress);
		this->connections.push_back(connection);

		this->secretToConnection[connection->secret] = connection;
		
		std::this_thread::sleep_for(std::chrono::milliseconds(16));
	}
}

void network::Network::openClient() {
	this->mode = NETWORK_CLIENT;
	this->client.open();
}

void network::Network::closeClient() {
	this->client.close();
}

void network::Network::tick() {
	Packet* packet = new Packet();
	packet->setType(DROPPABLE_PACKET);

	unsigned int packed = 0;
	for(RemoteObject* object: this->remoteObjects) {
		if(object->hasUpdate()) {
			packet->stream.startChunk();
			packet->stream.writeNumber<char>(REMOTE_OBJECT_UPDATE);
			object->pack(packet);
			packed++;
		}
	}

	if(packed == 0) {
		delete packet;
		return;
	}

	for(Connection* connection: this->connections) {
		connection->sendPacket(packet);
	}
}

void network::Network::receive() {
	if(this->isClient()) {
		this->client.receive();
		return;
	}
	
	#ifdef _WIN32
	this->udp.stream.allocate(EGGINE_PACKET_SIZE);
	
	for(unsigned int i = 0; i < 16; i++) {
		sockaddr_in6 clientAddress;
		socklen_t clientLength = sizeof(clientAddress);
		int length = recvfrom(this->udp.socket, &this->udp.stream.buffer[0], EGGINE_PACKET_SIZE, 0, (sockaddr*)&clientAddress, &clientLength);

		if(length <= 0) {
			if(WSAGetLastError() == WSAEWOULDBLOCK) {
				break;
			}
			continue;
		}

		// ## recv_generator.py "this->udp.stream" "length" "clientAddress"
	}
	#else
	for(unsigned int i = 0; i < EGGINE_NETWORK_UDP_MESSAGE_AMOUNT; i++) {
		this->udp.streams[i].allocate(EGGINE_PACKET_SIZE);
		this->udp.scatterGather[i].iov_base = (char*)this->udp.streams[i].start();
		this->udp.scatterGather[i].iov_len = EGGINE_PACKET_SIZE;
		this->udp.headers[i].msg_hdr.msg_iov = &this->udp.scatterGather[i];
		this->udp.headers[i].msg_hdr.msg_iovlen = 1;
		this->udp.headers[i].msg_hdr.msg_control = &this->udp.controls[i];
		this->udp.headers[i].msg_hdr.msg_controllen = sizeof(cmsghdr);
		this->udp.headers[i].msg_hdr.msg_flags = 0;
		this->udp.headers[i].msg_hdr.msg_name = &this->udp.addresses[i];
		this->udp.headers[i].msg_hdr.msg_namelen = sizeof(sockaddr_in6);
	}

	int messages = ::recvmmsg(this->udp.socket, this->udp.headers, EGGINE_NETWORK_UDP_MESSAGE_AMOUNT, 0, nullptr);
	if(messages > 0) {
		for(unsigned int i = 0; i < (unsigned int)messages; i++) {
			// ## recv_generator.py "this->udp.streams[i]" "this->udp.headers[i].msg_len" "this->udp.addresses[i]"
		}
	}
	#endif

	for(Connection* connection: this->connections) {
		connection->receiveTCP();
	}
}

void network::Network::addRemoteObject(RemoteObject* remoteObject) {
	remoteObject->remoteId = ++this->highestRemoteId;
	this->remoteObjects.push_back(remoteObject);
	this->idToRemoteObject[remoteObject->remoteId] = remoteObject;
}

void network::Network::removeRemoteObject(RemoteObject* remoteObject) {
	this->remoteObjects.erase(find(this->remoteObjects.begin(), this->remoteObjects.end(), remoteObject));
	this->idToRemoteObject[remoteObject->remoteId] = nullptr;
}

network::RemoteObject* network::Network::getRemoteObject(remote_object_id id) {
	return this->idToRemoteObject[id];
}

uint64_t network::Network::getUDPSocket() {
	return this->udp.socket;
}

bool network::Network::isServer() {
	return this->mode == NETWORK_SERVER;
}

bool network::Network::isClient() {
	return this->mode == NETWORK_CLIENT;
}

void network::Network::removeConnection(class Connection* connection) {
	this->connections.erase(find(this->connections.begin(), this->connections.end(), connection));
	
	if(connection->isInitialized()) {
		this->udpAddressToConnection[connection->udpAddress] = nullptr;
	}

	this->secretToConnection[connection->secret] = nullptr;
}

unsigned int network::Network::getConnectionCount() {
	return this->connections.size();
}

void network::Network::sendInitialData(Connection* connection) {
	Packet* packet = new Packet();
	packet->stream.setFlags(WRITE | NO_MASK_CHECKING);
	packet->setType(DROPPABLE_PACKET);
	for(RemoteObject* remoteObject: this->remoteObjects) {
		packet->stream.startChunk();
		packet->stream.writeNumber<char>(REMOTE_OBJECT_CREATE);
		remoteObject->pack(packet);
	}

	connection->sendPacket(packet);
}
