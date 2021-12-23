#pragma once

#include <string>

#include "../network/packet.h"
#include "../network/types.h"
#include "../network/stream.h"

namespace network {
	class RemoteObject {
		friend class Client;
		friend class Network;

		public:
			RemoteObject();
			~RemoteObject();
			
			virtual void pack(Packet* packet);
			virtual void unpack(Stream &stream);
			virtual unsigned int propertyToMaskPosition(std::string property);
			void writeUpdateMask(unsigned int position);
			void writeUpdateMask(std::string position);
			bool readUpdateMask(unsigned int position);
			void zeroUpdateMask();
			void printUpdateMask();
			bool hasUpdate();

			remote_object_id getRemoteId();
			virtual remote_class_id getRemoteClassId();
		
		protected:
			virtual void allocateMask();
			unsigned char* updateMask = nullptr; // variable size mask
			unsigned char updateMaskSize = 0;
			bool update = false;
			remote_object_id remoteId = 0;
	};
};
