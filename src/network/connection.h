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
			unsigned long secret = 0; // secret negociated with TCP, used to identify UDP ip/port
			int _socket = -1;
			IPAddress ip;

			void sendTCP(size_t size, const char* buffer);
			void sendUDP(size_t size, const char* buffer);
			void requestSecret();
			void initializeUDP(sockaddr_in6 address);
			void handlePacket();
	};
};
