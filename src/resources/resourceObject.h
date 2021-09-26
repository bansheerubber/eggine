#pragma once

#include <string>

using namespace std;

namespace resources {
	class ResourceObject { // keep track of resource usage via leasing
		public:
			ResourceObject(class ResourceManager* manager);
			
			void lease();
			void release();
			int getLeaseCount();
		
		protected:
			class ResourceManager* manager;
			int leases = 0;
	};
};
