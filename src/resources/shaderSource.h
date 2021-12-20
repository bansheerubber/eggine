#pragma once

#include "resourceObject.h"

namespace resources {
	class ShaderSource : public ResourceObject {
		public:
			ShaderSource(class ResourceManager* manager, carton::Metadata* metadata, const unsigned char* buffer, size_t bufferSize, bool useCharBuffer =  false);
			string source;
			unsigned char* buffer;
			size_t bufferSize = 0;
			ShaderSource* original = nullptr;

			void reload(carton::Metadata* metadata, const unsigned char* buffer, size_t bufferSize) {
				ResourceObject::reload(metadata, buffer, bufferSize);
				printf("shader source reload not implemented\n");
			}
			unsigned int getBytesUsed();
	};
};
