#pragma once

#include <string>

#include "../carton/metadata.h"

using namespace std;

namespace resources {
	class ResourceObject { // keep track of resource usage via leasing
		friend class ResourceManager;
		
		public:
			ResourceObject(class ResourceManager* manager, carton::Metadata* metadata);
			
			void lease();
			void release();
			int getLeaseCount();
			virtual void reload(carton::Metadata* metadata, const unsigned char* buffer, size_t bufferSize) {
				this->metadata = metadata;
			}
			virtual unsigned int getBytesUsed() = 0; // size of whatever buffer we're using for this resource object
		
		protected:
			class ResourceManager* manager;
			carton::Metadata* metadata = nullptr;
			string fileName;
			int leases = 0;
	};
};
