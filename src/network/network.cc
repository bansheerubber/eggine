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

bool network::Network::isActive() {
	return this->tcpSocket != -1;
}

void network::Network::open() {
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
		this->udpSocket = socket(AF_INET6, SOCK_DGRAM, 0);
		if(this->udpSocket < 0) {
			printf("could not instantiate udp socket\n");
			return;
		}

		int enabled = 1;
		setsockopt(this->udpSocket, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(int));

		if(bind(this->udpSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
			printf("could not bind tcp socket\n");
			return;
		}

		listen(this->udpSocket, 32);

		fcntl(this->udpSocket, F_SETFL, O_NONBLOCK);
	}

	thread t(&Network::accept, this);
	t.detach();
}

void network::Network::accept() {
	while(true) {
		sockaddr_in6 clientAddress;
		socklen_t clientLength = sizeof(clientAddress);
		int clientSocket = ::accept(this->tcpSocket, (sockaddr*)&clientAddress, &clientLength);
		if(clientSocket < 0) {
			this_thread::sleep_for(chrono::milliseconds(16));
			continue;
		}

		fcntl(clientSocket, F_SETFL, O_NONBLOCK);

		Connection* client = new Connection(clientSocket, clientAddress);
		this->clients.push_back(client);
		this->secretToConnection[client->secret] = client;
		thread t(&Network::sendInitialData, this, client);
		t.detach();

		this_thread::sleep_for(chrono::milliseconds(16));
	}
}

void network::Network::sendInitialData(Connection* connection) {
	Packet* packet = new Packet();
	packet->stream.setFlags(WRITE | NO_MASK_CHECKING);
	packet->setType(DROPPABLE_PACKET);
	for(RemoteObject* remoteObject: this->remoteObjects) {
		remoteObject->pack(packet);
	}

	connection->sendPacket(packet);
}

void network::Network::close() {

}

void network::Network::recv() {
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

	int messages = ::recvmmsg(this->udpSocket, this->udp.headers, EGGINE_NETWORK_UDP_MESSAGE_AMOUNT, 0, nullptr);
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
			}

			// the minimum header length for a packet is always greater than 8. if we got a length of 8, its probably a redundant secret
			if(this->udp.streams[i].buffer.head == 8) {
				continue;
			}

			this->udpAddressToConnection[this->udp.addresses[i]]->receiveUDP(this->udp.streams[i]);
		}
	}
	
	for(Connection* client: this->clients) {
		client->receiveTCP();
	}
}

void network::Network::tick() {
	if(getMicrosecondsNow() - this->frog > 300000) {
		for(Connection* client: this->clients) {
			if(!client->isInitialized()) {
				return;
			}

			Packet* packet = new Packet();
			packet->setType(DROPPABLE_PACKET);
			client->sendPacket(packet);
		}

		this->frog = getMicrosecondsNow();
	}
}

void network::Network::addRemoteObject(RemoteObject* remoteObject) {
	this->remoteObjects.push_back(remoteObject);
}

void network::Network::removeRemoteObject(RemoteObject* remoteObject) {
	this->remoteObjects.erase(find(this->remoteObjects.begin(), this->remoteObjects.end(), remoteObject));
}

int network::Network::getUDPSocket() {
	return this->udpSocket;
}