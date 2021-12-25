#pragma once

#include <vector>

#include "../util/dynamicArray.h"
#include "packetHandler.h"
#include "stream.h"

#define EGGINE_CLIENT_BUFFER_SIZE 65536

namespace network {
	class Client: public PacketHandler {
		public:
			Client();
			~Client();
			
			void open();
			void close();

			void receive();

			void sendPacket(Packet* packet);

			const IPAddress getIPAddress();
		
		protected:
			IPAddress ip;
			
			unsigned long secret = 0;
			bool initialized = false;
			bool hasSecret = false;
			
			int tcpSocket = -1;
			int udpSocket = -1;

			class RemoteObject* instantiateRemoteObject(remote_object_id remoteId, remote_class_id remoteClassId);
			void send(size_t size, const char* buffer);
			void receiveTCP();
			void receiveUDP();
			void handlePacket();
	};
};
