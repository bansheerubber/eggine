#include "resourceManager.h"

#include "pngImage.h"
#include "scriptFile.h"
#include "shaderSource.h"
#include "spriteSheet.h"

void handlePNGs(void* owner, carton::File* file, const char* buffer, size_t bufferSize) {
	((resources::ResourceManager*)owner)->metadataToResource[file->metadata]
		= new resources::SpriteSheet((resources::ResourceManager*)owner, file->metadata, (const unsigned char*)buffer, bufferSize);
}

void handleScripts(void* owner, carton::File* file, const char* buffer, size_t bufferSize) {
	((resources::ResourceManager*)owner)->metadataToResource[file->metadata]
		= new resources::ScriptFile((resources::ResourceManager*)owner, file->metadata, (const unsigned char*)buffer, bufferSize);
}

void handleShaders(void* owner, carton::File* file, const char* buffer, size_t bufferSize) {
	((resources::ResourceManager*)owner)->metadataToResource[file->metadata]
		= new resources::ShaderSource((resources::ResourceManager*)owner, file->metadata, (const unsigned char*)buffer, bufferSize);
}

resources::ResourceManager::ResourceManager(string fileName) {
	this->carton = new carton::Carton();
	this->carton->addExtensionHandler(".png", handlePNGs, this);
	this->carton->addExtensionHandler(".egg", handleScripts, this);
	this->carton->addExtensionHandler(".vert", handleShaders, this);
	this->carton->addExtensionHandler(".frag", handleShaders, this);
	this->carton->read(fileName);
}

DynamicArray<resources::ResourceObject*> resources::ResourceManager::loadResources(DynamicArray<carton::Metadata*> resources) {
	DynamicArray<ResourceObject*> output(resources.head);
	for(size_t i = 0; i < resources.head; i++) {
		carton::File* file = this->carton->readFile(resources[i]->getMetadata("fileName"));
		output[i] = this->metadataToResource[file->metadata];
	}
	return output;
}

DynamicArray<resources::ResourceObject*> resources::ResourceManager::metadataToResources(DynamicArray<carton::Metadata*> resources) {
	DynamicArray<ResourceObject*> output(resources.head);
	for(size_t i = 0; i < resources.head; i++) {
		output[i] = this->metadataToResource[resources[i]];
	}
	return output;
}
