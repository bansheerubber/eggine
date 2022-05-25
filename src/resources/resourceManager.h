#pragma once

#include <eggscript/egg.h>
#include <tsl/robin_set.h>
#include <string>

#include "../carton/carton.h"
#include "../util/dynamicArray.h"
#include "../carton/file.h"
#include "../carton/metadata.h"
#include "../carton/metadata/queryList.h"

namespace es {
	void defineResourceManager();
	esEntryPtr hotReload(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
};

void handleSpritesheets(void* owner, carton::File* file, const char* buffer, uint64_t bufferSize);
void handlePNGs(void* owner, carton::File* file, const char* buffer, uint64_t bufferSize);
void handleHTML(void* owner, carton::File* file, const char* buffer, uint64_t bufferSize);
void handleCSS(void* owner, carton::File* file, const char* buffer, uint64_t bufferSize);
void handleScripts(void* owner, carton::File* file, const char* buffer, uint64_t bufferSize);
void handleShaders(void* owner, carton::File* file, const char* buffer, uint64_t bufferSize);
void handleDKSHShaders(void* owner, carton::File* file, const char* buffer, uint64_t bufferSize);
void handleSPVShaders(void* owner, carton::File* file, const char* buffer, uint64_t bufferSize);
void handleMaps(void* owner, carton::File* file, const char* buffer, uint64_t bufferSize);
void handleSounds(void* owner, carton::File* file, const char* buffer, uint64_t bufferSize);

namespace resources {
	class ResourceManager {
		friend class ResourceObject;
		friend void ::handleSpritesheets(void* owner, carton::File* file, const char* buffer, uint64_t bufferSize);
		friend void ::handlePNGs(void* owner, carton::File* file, const char* buffer, uint64_t bufferSize);
		friend void ::handleHTML(void* owner, carton::File* file, const char* buffer, uint64_t bufferSize);
		friend void ::handleCSS(void* owner, carton::File* file, const char* buffer, uint64_t bufferSize);
		friend void ::handleScripts(void* owner, carton::File* file, const char* buffer, uint64_t bufferSize);
		friend void ::handleShaders(void* owner, carton::File* file, const char* buffer, uint64_t bufferSize);
		friend void ::handleDKSHShaders(void* owner, carton::File* file, const char* buffer, uint64_t bufferSize);
		friend void ::handleSPVShaders(void* owner, carton::File* file, const char* buffer, uint64_t bufferSize);
		friend void ::handleMaps(void* owner, carton::File* file, const char* buffer, uint64_t bufferSize);
		friend void ::handleSounds(void* owner, carton::File* file, const char* buffer, uint64_t bufferSize);
		
		public:
			ResourceManager();
			~ResourceManager();
			
			carton::Carton* carton = nullptr;
			std::string fileName;
			DynamicArray<class ResourceObject*> loadResources(DynamicArray<carton::Metadata*> resources);
			DynamicArray<class ResourceObject*> metadataToResources(DynamicArray<carton::Metadata*> resources);
			unsigned int getBytesUsed();
			void reload();
			void tick();

			void init(std::string fileName);
		
		private:
			#ifdef __linux__
			int inotify = -1;
			int watch = -1;
			uint64_t lastEvent = 0;
			carton::CartonHash hash;
			bool hashed = false;
			#endif

			tsl::robin_set<class ResourceObject*> objects;
			tsl::robin_map<carton::Metadata*, class ResourceObject*> metadataToResource;
	};

	class ShaderSource;
};

resources::ShaderSource* getShaderSource(std::string fileName);
