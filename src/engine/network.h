#pragma once

#include <vector>

#include "networkStream.h"

using namespace std;

namespace network {
	class Network {
		public:
			void tick();
			
			void addRemoteObject(class RemoteObject* remoteObject);
			void removeRemoteObject(class RemoteObject* remoteObject);
			void send(Stream &stream);
		
		private:
			vector<class RemoteObject*> remoteObjects;
	};
};
