#include "resourceManager.h"

#include "pngImage.h"

void handlePNGs(void* owner, carton::File* file, const char* buffer, size_t bufferSize) {
	((resources::ResourceManager*)owner)->metadataToResource[file->metadata]
		= new resources::PNGImage((resources::ResourceManager*)owner, (const unsigned char*)buffer, bufferSize);
}

resources::ResourceManager::ResourceManager(string fileName) {
	this->carton = new carton::Carton();
	this->carton->addExtensionHandler(".png", handlePNGs, this);
	this->carton->read(fileName);
}

void resources::ResourceManager::loadResources(DynamicArray<carton::Metadata*> resources) {
	for(size_t i = 0; i < resources.head; i++) {
		this->carton->readFile(resources[i]->getMetadata("fileName"));
	}
}

DynamicArray<resources::ResourceObject*> resources::ResourceManager::metadataToResources(DynamicArray<carton::Metadata*> resources) {
	DynamicArray<ResourceObject*> output(resources.head);
	for(size_t i = 0; i < resources.head; i++) {
		output[i] = this->metadataToResource[resources[i]];
	}
	return output;
}
