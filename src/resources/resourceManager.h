#pragma once

#include <tsl/robin_set.h>
#include <string>

#include "../carton/carton.h"
#include "../util/dynamicArray.h"
#include "../carton/file.h"
#include "../carton/metadata.h"
#include "../carton/metadata/queryList.h"
#include "resourceObject.h"

void handlePNGs(void* owner, carton::File* file, const char* buffer, size_t bufferSize);
void handleScripts(void* owner, carton::File* file, const char* buffer, size_t bufferSize);
void handleShaders(void* owner, carton::File* file, const char* buffer, size_t bufferSize);
void handleDKSHShaders(void* owner, carton::File* file, const char* buffer, size_t bufferSize);
void handleMaps(void* owner, carton::File* file, const char* buffer, size_t bufferSize);

namespace resources {
	class ResourceManager {
		friend ResourceObject;
		friend void ::handlePNGs(void* owner, carton::File* file, const char* buffer, size_t bufferSize);
		friend void ::handleScripts(void* owner, carton::File* file, const char* buffer, size_t bufferSize);
		friend void ::handleShaders(void* owner, carton::File* file, const char* buffer, size_t bufferSize);
		friend void ::handleDKSHShaders(void* owner, carton::File* file, const char* buffer, size_t bufferSize);
		friend void ::handleMaps(void* owner, carton::File* file, const char* buffer, size_t bufferSize);
		
		public:
			ResourceManager(string fileName);
			
			carton::Carton* carton = nullptr;
			DynamicArray<ResourceObject*> loadResources(DynamicArray<carton::Metadata*> resources);
			DynamicArray<ResourceObject*> metadataToResources(DynamicArray<carton::Metadata*> resources);
		
		private:
			tsl::robin_set<ResourceObject*> objects;
			tsl::robin_map<carton::Metadata*, ResourceObject*> metadataToResource;
	};

	class ShaderSource;
};

resources::ShaderSource* getShaderSource(string fileName);
