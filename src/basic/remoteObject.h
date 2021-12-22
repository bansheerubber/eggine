#pragma once

#include <string>

#include "../engine/networkStream.h"

using namespace std;

namespace network {
	class RemoteObject {
		public:
			RemoteObject();
			~RemoteObject();
			
			virtual void pack(Stream &stream);
			virtual unsigned int propertyToMaskPosition(string property);
			void writeUpdateMask(unsigned int position);
			void writeUpdateMask(string position);
			bool readUpdateMask(unsigned int position);
			void zeroUpdateMask();
			void printUpdateMask();
			bool hasUpdate();
		
		protected:
			virtual void allocateMask();
			char* updateMask = nullptr; // variable size mask
			unsigned int updateMaskSize = 0;
			bool update = false;
			unsigned int remoteId = 0;
	};
};
