#pragma once

#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <netinet/in.h>

#include "../util/dynamicArray.h"
#include "packetHandler.h"
#include "stream.h"

#define EGGINE_CONNECTION_BUFFER_SIZE 65536

namespace network {
	enum ConnectionHandshakeState {
		INVALID_STATE,
		WAIT_FOR_CHECKSUM,
		WAIT_FOR_SECRET,
		PACKET_READY
	};
	
	struct ConnectionIPAddress {
		unsigned char address[16];

		ConnectionIPAddress() {}
		ConnectionIPAddress(sockaddr_in6 address) {
			for(unsigned int i = 0; i < 16; i++) {
				this->address[i] = address.sin6_addr.s6_addr[i];
			}
		}

		std::string toString() {
			return fmt::format(
				"{:x}{:x}:{:x}{:x}:{:x}{:x}:{:x}{:x}:{:x}{:x}:{:x}{:x}:{:x}{:x}:{:x}{:x}",
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
				this->address[15]
			);
		}
	};
	
	class Connection: public PacketHandler {
		friend class Network;
		
		public:
			Connection(int _socket, sockaddr_in6 address);
			~Connection();

			void receiveTCP();
			void receiveUDP(Stream &stream);

			void sendPacket(Packet* packet);

			bool isInitialized();
		
		protected:			
			ConnectionHandshakeState handshake = WAIT_FOR_CHECKSUM;
			
			sockaddr_in6 tcpAddress;
			sockaddr_in6 udpAddress;

			bool initialized = false;
			unsigned long secret = 2974321; // secret negociated with TCP, used to identify UDP ip/port
			int _socket = -1;
			ConnectionIPAddress ip;

			void sendTCP(size_t size, const char* buffer);
			void sendUDP(size_t size, const char* buffer);
			void requestSecret();
			void initializeUDP(sockaddr_in6 address);
			void handlePacket();
	};
};
