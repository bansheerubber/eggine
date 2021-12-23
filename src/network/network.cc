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
		printf("got a message!\n");
		for(unsigned int i = 0; i < messages; i++) {
			// prepare the buffers
			this->udp.streams[i].flush();
			this->udp.streams[i].buffer.head = this->udp.headers[i].msg_len;
			
			printf("and its %d bytes long!!\n", this->udp.headers[i].msg_len);
			printf("%.16s\n", this->udp.headers[i].msg_hdr.msg_iov->iov_base);

			printf("%s\n", ipv6ToString((sockaddr_in6*)this->udp.headers[i].msg_hdr.msg_name).c_str());

			const char response[17] = "frog to meet you";
			::sendto(this->udpSocket, response, 16, 0, (sockaddr*)&this->udp.addresses[i], sizeof(sockaddr_in6));
		}
	}
	
	for(Connection* client: this->clients) {
		client->receiveTCP();
	}
}

void network::Network::tick() {
	if(getMicrosecondsNow() - this->frog > 300000) {
		for(Connection* client: this->clients) {
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
