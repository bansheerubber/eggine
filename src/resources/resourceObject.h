#pragma once

#include <string>

#include "../carton/metadata.h"
#include "resourceManager.h"

namespace resources {
	class ResourceObject { // keep track of resource usage via leasing
		friend class ResourceManager;
		
		public:
			ResourceObject(ResourceManager &manager, carton::Metadata* metadata);
			
			void lease();
			void release();
			int getLeaseCount();
			virtual void reload(carton::Metadata* metadata, const unsigned char* buffer, uint64_t bufferSize) {
				this->metadata = metadata;
			}
			virtual unsigned int getBytesUsed() = 0; // size of whatever buffer we're using for this resource object
		
		protected:
			class ResourceManager* manager;
			carton::Metadata* metadata = nullptr;
			std::string fileName;
			int leases = 0;
	};
};
