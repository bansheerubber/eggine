#pragma once

#include <sys/socket.h>
#include <vector>

#include "stream.h"
#include "types.h"

namespace network {
	#define EGGINE_NETWORK_UDP_MESSAGE_AMOUNT 32
	
	class Network {
		public:
			Network();
			~Network();

			bool isActive();

			void open();
			void close();
			void accept();

			void sendInitialData(class Connection* connection);

			void tick();
			void recv();
			
			void addRemoteObject(class RemoteObject* remoteObject);
			void removeRemoteObject(class RemoteObject* remoteObject);
		
		private:
			int tcpSocket = -1;
			int udpSocket = -1;
			std::vector<class RemoteObject*> remoteObjects;
			std::vector<class Connection*> clients;
			Stream udpStreams[EGGINE_NETWORK_UDP_MESSAGE_AMOUNT];
			iovec scatterGather[32];
			cmsghdr controls[32];

			unsigned long long frog;
	};
};
