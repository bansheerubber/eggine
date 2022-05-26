#pragma once

#include "resourceObject.h"

namespace resources {
	class TextFile: public ResourceObject {
		public:
			TextFile(ResourceManager &manager, carton::Metadata* metadata, const unsigned char* buffer, uint64_t bufferSize);

			void reload(carton::Metadata* metadata, const unsigned char* buffer, uint64_t bufferSize);
			unsigned int getBytesUsed();

			std::string getSource();
		
		private:
			std::string source;
	};
};
