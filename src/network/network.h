#pragma once

#include <vector>

#include "stream.h"
#include "types.h"

namespace network {
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
			int _socket = -1;
			std::vector<class RemoteObject*> remoteObjects;
			std::vector<class Connection*> clients;

			unsigned long long frog;
	};
};
