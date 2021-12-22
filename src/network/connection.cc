#include "connection.h"

#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

network::Connection::Connection(int _socket, sockaddr_in6 address) {
	this->_socket = _socket;
	this->address = address;
	this->ip = ConnectionIPAddress(address);
}

void network::Connection::send(size_t size, const char* buffer) {
	::send(this->_socket, buffer, size, 0);
}

void network::Connection::recv() {
	int length = ::recv(this->_socket, &this->buffer[0], EGGINE_CONNECTION_BUFFER_SIZE, 0);
	if(length < 0) {
		if(errno == EWOULDBLOCK) {
			return;
		}
	}
	else if(length == 0) {
		
	}
}
