#pragma once

#include <string>

#include "resourceObject.h"

using namespace std;

namespace resources {
	class ScriptFile: public ResourceObject {
		public:
			ScriptFile(class ResourceManager* manager, carton::Metadata* metadata, const unsigned char* buffer, size_t bufferSize);
			string script;

			unsigned int getBytesUsed();
	};
};