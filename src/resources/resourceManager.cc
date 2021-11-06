#include "resourceManager.h"

#include "../engine/developer.h"

#include "../test/developerGui.h"
#include "../engine/engine.h"
#include "../util/png.h"
#include "../util/crop.h"
#include "scriptFile.h"
#include "shaderSource.h"
#include "spriteSheet.h"
#include "../renderer/texture.h"
#include "../test/tileMath.h"

void handlePNGs(void* owner, carton::File* file, const char* buffer, size_t bufferSize) {
	((resources::ResourceManager*)owner)->metadataToResource[file->metadata]
		= new resources::SpriteSheet((resources::ResourceManager*)owner, file->metadata, (const unsigned char*)buffer, bufferSize);

	#ifdef EGGINE_DEVELOPER_MODE
	if(file->getFileName() == "spritesheets/spritesheet.png") {
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
	this->carton->addExtensionHandler(".png", handlePNGs, this);
	this->carton->addExtensionHandler(".egg", handleScripts, this);
	this->carton->addExtensionHandler(".vert", handleShaders, this);
	this->carton->addExtensionHandler(".frag", handleShaders, this);
	this->carton->addExtensionHandler(".dksh", handleDKSHShaders, this);
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
	output[0] = nullptr;
	for(size_t i = 0; i < resources.head; i++) {
		output[i] = this->metadataToResource[resources[i]];
	}
	return output;
}
