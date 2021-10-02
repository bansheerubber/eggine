#pragma once

#include "resourceObject.h"

namespace resources {
	class ShaderSource : public ResourceObject {
		public:
			ShaderSource(class ResourceManager* manager, carton::Metadata* metadata, const unsigned char* buffer, size_t bufferSize);
			string source;
	};
};
