#pragma once

#include <string>

#include "resourceObject.h"

using namespace std;

namespace resources {
	class ScriptFile: public ResourceObject {
		public:
			ScriptFile(class ResourceManager* manager, carton::Metadata* metadata, const unsigned char* buffer, uint64_t bufferSize);
			string script;

			void reload(carton::Metadata* metadata, const unsigned char* buffer, uint64_t bufferSize);
			unsigned int getBytesUsed();
	};
};