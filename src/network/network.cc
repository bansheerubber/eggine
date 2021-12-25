#include "network.h"

#include <algorithm>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <sys/types.h>

#include "connection.h"
#include "../util/ipv6ToString.h"
#include "packet.h"
#include "../basic/remoteObject.h"

#include "../util/time.h"

using namespace std;

network::Network::Network() {
}

network::Network::~Network() {
}

void network::Network::openServer() {
	this->mode = NETWORK_SERVER;
	
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

	thread t(&Network::acceptServer, this);
	t.detach();
}

void network::Network::closeServer() {

}

void network::Network::acceptServer() {
	while(true) {
		sockaddr_in6 clientAddress;
		socklen_t clientLength = sizeof(clientAddress);
		int clientSocket = ::accept(this->tcpSocket, (sockaddr*)&clientAddress, &clientLength);
		if(clientSocket < 0) {
			this_thread::sleep_for(chrono::milliseconds(16));
			continue;
		}

		fcntl(clientSocket, F_SETFL, O_NONBLOCK);

		Connection* connection = new Connection(clientSocket, clientAddress);
		this->connections.push_back(connection);
		this->secretToConnection[connection->secret] = connection;

		this_thread::sleep_for(chrono::milliseconds(16));
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
		for(unsigned int i = 0; i < messages; i++) {
			// prepare the buffers
			this->udp.streams[i].flush();
			this->udp.streams[i].buffer.head = this->udp.headers[i].msg_len;

			if(this->udp.streams[i].buffer.head < 8) { // if the message isn't even big enough to hold a secret, discard the packet
				printf("discard because of small length\n");
				continue;
			}
			
			if(
				this->udpAddressToConnection.find(this->udp.addresses[i]) == this->udpAddressToConnection.end()
			) {
				// if we can't find a secret, discard the packet
				unsigned long secret = this->udp.streams[i].readNumber<unsigned long>();
				if(this->secretToConnection.find(secret) == this->secretToConnection.end()) {
					printf("discard because of invalid secret\n");
					continue;
				}

				this->udpAddressToConnection[this->udp.addresses[i]] = this->secretToConnection[secret];
				this->secretToConnection[secret]->initializeUDP(this->udp.addresses[i]);

				thread t(&Network::sendInitialData, this, this->secretToConnection[secret]);
				t.detach();
			}

			// the minimum header length for a packet is always greater than 8. if we got a length of 8, its probably a redundant secret
			if(this->udp.streams[i].buffer.head == 8) {
				continue;
			}

			this->udpAddressToConnection[this->udp.addresses[i]]->receiveUDP(this->udp.streams[i]);
		}
	}
	
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

int network::Network::getUDPSocket() {
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
