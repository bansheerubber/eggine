#pragma once

#include <string>

#include "../network/stream.h"

namespace network {
	class RemoteObject {
		public:
			RemoteObject();
			~RemoteObject();
			
			virtual void pack(Stream &stream);
			virtual unsigned int propertyToMaskPosition(std::string property);
			void writeUpdateMask(unsigned int position);
			void writeUpdateMask(std::string position);
			bool readUpdateMask(unsigned int position);
			void zeroUpdateMask();
			void printUpdateMask();
			bool hasUpdate();

			unsigned long getRemoteId();
			virtual unsigned short getRemoteClassId();
		
		protected:
			virtual void allocateMask();
			char* updateMask = nullptr; // variable size mask
			unsigned int updateMaskSize = 0;
			bool update = false;
			unsigned long remoteId = 0;
	};
};
