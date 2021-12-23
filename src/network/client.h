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

			bool isActive();
			
			void open();
			void close();

			void tick();

			void addRemoteObject(class RemoteObject* remoteObject);
			void removeRemoteObject(class RemoteObject* remoteObject);

			void sendPacket(Packet* packet);
		
		private:
			int tcpSocket = -1;
			int udpSocket = -1;
			std::vector<class RemoteObject*> remoteObjects;

			void instantiateRemoteObject(unsigned long remoteId, unsigned short remoteClassId);
			void send(size_t size, const char* buffer);
	};
};
