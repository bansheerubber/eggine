#pragma once

#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <netinet/in.h>

#include "../util/dynamicArray.h"
#include "packetHandler.h"
#include "stream.h"

#define EGGINE_CONNECTION_BUFFER_SIZE 65536

namespace network {
	struct ConnectionIPAddress {
		sa_family_t _;
		unsigned short port;
		uint32_t __;
		unsigned char address[16];

		ConnectionIPAddress() {}
		ConnectionIPAddress(sockaddr_in6 address) {
			for(unsigned int i = 0; i < 16; i++) {
				this->address[i] = address.sin6_addr.s6_addr[i];
			}
			this->port = address.sin6_port;
		}

		std::string toString() {
			return fmt::format(
				"{:x}{:x}:{:x}{:x}:{:x}{:x}:{:x}{:x}:{:x}{:x}:{:x}{:x}:{:x}{:x}:{:x}{:x}:{}",
				this->address[0],
				this->address[1],
				this->address[2],
				this->address[3],
				this->address[4],
				this->address[5],
				this->address[6],
				this->address[7],
				this->address[8],
				this->address[9],
				this->address[10],
				this->address[11],
				this->address[12],
				this->address[13],
				this->address[14],
				this->address[15],
				this->port
			);
		}
	};
	
	class Connection: public PacketHandler {
		public:
			Connection(int _socket, sockaddr_in6 address);

			void receiveTCP();
			void receiveUDP(Stream &stream);

			void sendPacket(Packet* packet);
		
		protected:			
			sockaddr_in6 address;

			int _socket;
			ConnectionIPAddress ip;

			void send(size_t size, const char* buffer);
	};
};
