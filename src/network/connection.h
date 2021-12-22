#pragma once

#include <netinet/in.h>

#include "../util/dynamicArray.h"
#include "stream.h"

#define EGGINE_CONNECTION_BUFFER_SIZE 65536

namespace network {
	struct ConnectionIPAddress {
		unsigned char address[16];
		unsigned short port;

		ConnectionIPAddress() {}
		ConnectionIPAddress(sockaddr_in6 address) {
			for(unsigned int i = 0; i < 16; i++) {
				this->address[i] = address.sin6_addr.s6_addr[i];
			}
			this->port = address.sin6_port;
		}
	};
	
	class Connection {
		public:
			Connection(int _socket, sockaddr_in6 address);

			void recv();
			void send(size_t size, const char* buffer);
		
		protected:			
			sockaddr_in6 address;

			int _socket;
			ConnectionIPAddress ip;

			DynamicArray<char> buffer = DynamicArray<char>(EGGINE_CONNECTION_BUFFER_SIZE);
	};
};
