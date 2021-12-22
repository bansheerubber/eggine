#pragma once

#include <vector>

#include "../util/dynamicArray.h"
#include "stream.h"

#define EGGINE_CLIENT_BUFFER_SIZE 65536

namespace network {
	class Client {
		public:
			Client();
			~Client();

			bool isActive();
			
			void open();
			void close();

			void tick();

			void addRemoteObject(class RemoteObject* remoteObject);
			void removeRemoteObject(class RemoteObject* remoteObject);
			void send(size_t size, const char* buffer);
		
		private:
			int _socket = -1;
			std::vector<class RemoteObject*> remoteObjects;

			DynamicArray<char> buffer = DynamicArray<char>(EGGINE_CLIENT_BUFFER_SIZE);
			Stream stream = Stream(READ);

			void instantiateRemoteObject(unsigned int remoteClassId);
	};
};
