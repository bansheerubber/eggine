#include "resourceManager.h"

#include "../engine/developer.h"

#include "../util/crop.h"
#include "css.h"
#include "../test/developerGui.h"
#include "../engine/engine.h"
#include "html.h"
#include "image.h"
#include "mapSource.h"
#include "../util/png.h"
#include "scriptFile.h"
#include "shaderSource.h"
#include "../sound/soundCollection.h"
#include "spriteSheet.h"
#include "../renderer/texture.h"
#include "../test/tileMath.h"

void handleSpritesheets(void* owner, carton::File* file, const char* buffer, size_t bufferSize) {
	((resources::ResourceManager*)owner)->metadataToResource[file->metadata]
		= new resources::SpriteSheet((resources::ResourceManager*)owner, file->metadata, (const unsigned char*)buffer, bufferSize);
	
	#ifdef EGGINE_DEVELOPER_MODE
	if(file->getFileName() == "spritesheets/spritesheet.ss") {
		engine->developerGui->spritesheet = loadPng((unsigned char*)buffer, bufferSize);
		for(unsigned int i = 0; i < stoi(file->metadata->getMetadata("amount")); i++) {
			glm::ivec2 start = tilemath::textureIndexToXY(i, 1057, 391);
			cropped result = crop(engine->developerGui->spritesheet, start.x, start.y, 64, 128);

			render::Texture* texture = new render::Texture(&engine->renderWindow);
			texture->setFilters(render::TEXTURE_FILTER_NEAREST, render::TEXTURE_FILTER_NEAREST);
			texture->setWrap(render::TEXTURE_WRAP_CLAMP_TO_BORDER, render::TEXTURE_WRAP_CLAMP_TO_BORDER);
			texture->load(result.buffer, result.bufferSize, result.width, result.height, result.source.bitDepth, result.source.channels);

			engine->developerGui->spritesheetImages.push_back(texture);

			delete[] result.buffer;
		}
	}
	#endif
}

void handleHTML(void* owner, carton::File* file, const char* buffer, size_t bufferSize) {
	((resources::ResourceManager*)owner)->metadataToResource[file->metadata]
		= new resources::HTML((resources::ResourceManager*)owner, file->metadata, (const unsigned char*)buffer, bufferSize);
}

void handleCSS(void* owner, carton::File* file, const char* buffer, size_t bufferSize) {
	((resources::ResourceManager*)owner)->metadataToResource[file->metadata]
		= new resources::CSS((resources::ResourceManager*)owner, file->metadata, (const unsigned char*)buffer, bufferSize);
}

void handlePNGs(void* owner, carton::File* file, const char* buffer, size_t bufferSize) {
	((resources::ResourceManager*)owner)->metadataToResource[file->metadata]
		= new resources::Image((resources::ResourceManager*)owner, file->metadata, (const unsigned char*)buffer, bufferSize);
}

void handleScripts(void* owner, carton::File* file, const char* buffer, size_t bufferSize) {
	((resources::ResourceManager*)owner)->metadataToResource[file->metadata]
		= new resources::ScriptFile((resources::ResourceManager*)owner, file->metadata, (const unsigned char*)buffer, bufferSize);
}

void handleShaders(void* owner, carton::File* file, const char* buffer, size_t bufferSize) {
	((resources::ResourceManager*)owner)->metadataToResource[file->metadata]
		= new resources::ShaderSource((resources::ResourceManager*)owner, file->metadata, (const unsigned char*)buffer, bufferSize);
}

void handleDKSHShaders(void* owner, carton::File* file, const char* buffer, size_t bufferSize) {
	((resources::ResourceManager*)owner)->metadataToResource[file->metadata]
		= new resources::ShaderSource((resources::ResourceManager*)owner, file->metadata, (const unsigned char*)buffer, bufferSize, true);
	
	// find the .vert/.frag and associate it with the shader source so we can get uniform buffer bindings
	resources::ShaderSource* original = (resources::ShaderSource*)((resources::ResourceManager*)owner)->metadataToResources(
		((resources::ResourceManager*)owner)->carton->database.get()->equals(
			"fileName",
			file->getFileName().replace(file->getFileName().size() - 5, file->getFileName().size(), "")
		)->exec()
	)[0];
	((resources::ShaderSource*)(((resources::ResourceManager*)owner)->metadataToResource[file->metadata]))->original = original;
}

void handleMaps(void* owner, carton::File* file, const char* buffer, size_t bufferSize) {
	((resources::ResourceManager*)owner)->metadataToResource[file->metadata]
		= new resources::MapSource((resources::ResourceManager*)owner, file->metadata, (const unsigned char*)buffer, bufferSize);
}

void handleSounds(void* owner, carton::File* file, const char* buffer, size_t bufferSize) {
	((resources::ResourceManager*)owner)->metadataToResource[file->metadata]
		= new sound::SoundCollection((resources::ResourceManager*)owner, file->metadata, (const unsigned char*)buffer, bufferSize);
}

resources::ShaderSource* getShaderSource(string fileName) {
	#ifdef __switch__
	fileName += ".dksh";
	#endif
	
	return (resources::ShaderSource*)engine->manager->metadataToResources(
		engine->manager->carton->database.get()->equals("fileName", fileName)->exec()
	)[0];
}

resources::ResourceManager::ResourceManager(string fileName) {
	this->carton = new carton::Carton();
	this->carton->addExtensionHandler(".ss", handleSpritesheets, this);
	this->carton->addExtensionHandler(".png", handlePNGs, this);
	this->carton->addExtensionHandler(".html", handleHTML, this);
	this->carton->addExtensionHandler(".css", handleCSS, this);
	this->carton->addExtensionHandler(".egg", handleScripts, this);
	this->carton->addExtensionHandler(".vert", handleShaders, this);
	this->carton->addExtensionHandler(".frag", handleShaders, this);
	this->carton->addExtensionHandler(".dksh", handleDKSHShaders, this);
	this->carton->addExtensionHandler(".map", handleMaps, this);
	this->carton->addExtensionHandler(".sound", handleSounds, this);
	this->carton->read(fileName);
}

unsigned int resources::ResourceManager::getBytesUsed() {
	unsigned int total = 0;
	for(ResourceObject* object: this->objects) {
		total += object->getBytesUsed();
	}
	return total;
}

DynamicArray<resources::ResourceObject*> resources::ResourceManager::loadResources(DynamicArray<carton::Metadata*> resources) {
	DynamicArray<ResourceObject*> output(resources.head);
	for(size_t i = 0; i < resources.head; i++) {
		if(resources[i]->getMetadata("stream") == "true") {
			printf("Cannot load streamed file '%s' into memory\n", resources[i]->getMetadata("fileName").c_str());
			continue;
		}

		carton::File* file = this->carton->readFile(resources[i]->getMetadata("fileName"));
		output[i] = this->metadataToResource[file->metadata];
	}
	return output;
}

DynamicArray<resources::ResourceObject*> resources::ResourceManager::metadataToResources(DynamicArray<carton::Metadata*> resources) {
	DynamicArray<ResourceObject*> output(resources.head);
	output[0] = nullptr;
	for(size_t i = 0; i < resources.head; i++) {
		output[i] = this->metadataToResource[resources[i]];
	}
	return output;
}
