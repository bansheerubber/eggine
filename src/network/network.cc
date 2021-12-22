#include "network.h"

#include <algorithm>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <sys/types.h>

#include "connection.h"
#include "../basic/remoteObject.h"

using namespace std;

network::Network::Network() {
}

network::Network::~Network() {
}

bool network::Network::isActive() {
	return this->_socket != -1;
}

void network::Network::open() {
	this->_socket = socket(AF_INET6, SOCK_STREAM, 0);
	if(this->_socket < 0) {
		printf("could not open socket\n");
		return;
	}

	sockaddr_in6 serverAddress;

	serverAddress.sin6_family = AF_INET6;
	serverAddress.sin6_addr = in6addr_any;
	serverAddress.sin6_port = htons(28000);

	int enabled = 1;
	setsockopt(this->_socket, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(int));

	if(bind(this->_socket, (sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
		printf("could not bind socket\n");
		return;
	}

	listen(this->_socket, 32);

	fcntl(this->_socket, F_SETFL, O_NONBLOCK);

	thread t(&Network::accept, this);
	t.detach();
}

void network::Network::accept() {
	while(true) {
		sockaddr_in6 clientAddress;
		socklen_t clientLength = sizeof(clientAddress);
		int clientSocket = ::accept(this->_socket, (sockaddr*)&clientAddress, &clientLength);
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
	Stream stream(WRITE | NO_MASK_CHECKING);
	for(RemoteObject* remoteObject: this->remoteObjects) {
		remoteObject->pack(stream);
		stream.send(connection);
	}
}

void network::Network::close() {

}

void network::Network::recv() {
	for(Connection* client: this->clients) {
		client->recv();
	}
}

void network::Network::tick() {
	// Stream stream;
	// for(RemoteObject* remoteObject: this->remoteObjects) {
	// 	if(remoteObject->hasUpdate()) {
	// 		remoteObject->pack(stream);
	// 	}
	// }
}

void network::Network::addRemoteObject(RemoteObject* remoteObject) {
	this->remoteObjects.push_back(remoteObject);
}

void network::Network::removeRemoteObject(RemoteObject* remoteObject) {
	this->remoteObjects.erase(find(this->remoteObjects.begin(), this->remoteObjects.end(), remoteObject));
}
