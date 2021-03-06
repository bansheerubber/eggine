#pragma once

#include "../engine/console.h"
#include "resourceObject.h"

namespace resources {
	class ShaderSource : public ResourceObject {
		public:
			ShaderSource(ResourceManager &manager, carton::Metadata* metadata, const unsigned char* buffer, uint64_t bufferSize);
			~ShaderSource();

			std::string fileName;
			unsigned char* buffer;
			uint64_t bufferSize = 0;
			ShaderSource* original = nullptr;

			void reload(carton::Metadata* metadata, const unsigned char* buffer, uint64_t bufferSize) {
				ResourceObject::reload(metadata, buffer, bufferSize);
				console::warning("shader source reload not implemented\n");
			}
			unsigned int getBytesUsed();
	};
};
