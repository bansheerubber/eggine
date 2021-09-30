#pragma once

#include <string>

#include "../carton/metadata.h"

using namespace std;

namespace resources {
	class ResourceObject { // keep track of resource usage via leasing
		public:
			ResourceObject(class ResourceManager* manager, carton::Metadata* metadata);
			
			void lease();
			void release();
			int getLeaseCount();
		
		protected:
			class ResourceManager* manager;
			carton::Metadata* metadata = nullptr;
			int leases = 0;
	};
};
