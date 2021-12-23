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
		
		protected:
			unsigned long secret = 0;
			bool initialized = false;
			
			int tcpSocket = -1;
			int udpSocket = -1;
			std::vector<class RemoteObject*> remoteObjects;

			class RemoteObject* instantiateRemoteObject(remote_object_id remoteId, remote_class_id remoteClassId);
			void send(size_t size, const char* buffer);
			void receiveTCP();
			void receiveUDP();
			void handlePacket();
	};
};
