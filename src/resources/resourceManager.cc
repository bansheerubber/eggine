#include "resourceManager.h"


#ifdef __linux__
#include <fcntl.h>
#include <sys/inotify.h>
#include <unistd.h>

#include "../util/md5hash.h"
#endif

#include "../engine/developer.h"

#include "../engine/console.h"
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

void handleSpritesheets(void* owner, carton::File* file, const char* buffer, uint64_t bufferSize) {
	((resources::ResourceManager*)owner)->metadataToResource[file->metadata]
		= new resources::SpriteSheet((resources::ResourceManager*)owner, file->metadata, (const unsigned char*)buffer, bufferSize);
	
	#ifdef EGGINE_DEVELOPER_MODE
	if(file->getFileName() == "spritesheets/spritesheet.ss") {
		engine->developerGui->spritesheet = loadPng((unsigned char*)buffer, bufferSize);
		for(unsigned int i = 0; i < stoul(file->metadata->getMetadata("amount")); i++) {
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

void handleHTML(void* owner, carton::File* file, const char* buffer, uint64_t bufferSize) {
	((resources::ResourceManager*)owner)->metadataToResource[file->metadata]
		= new resources::HTML((resources::ResourceManager*)owner, file->metadata, (const unsigned char*)buffer, bufferSize);
}

void handleCSS(void* owner, carton::File* file, const char* buffer, uint64_t bufferSize) {
	((resources::ResourceManager*)owner)->metadataToResource[file->metadata]
		= new resources::CSS((resources::ResourceManager*)owner, file->metadata, (const unsigned char*)buffer, bufferSize);
}

void handlePNGs(void* owner, carton::File* file, const char* buffer, uint64_t bufferSize) {
	((resources::ResourceManager*)owner)->metadataToResource[file->metadata]
		= new resources::Image((resources::ResourceManager*)owner, file->metadata, (const unsigned char*)buffer, bufferSize);
}

void handleScripts(void* owner, carton::File* file, const char* buffer, uint64_t bufferSize) {
	((resources::ResourceManager*)owner)->metadataToResource[file->metadata]
		= new resources::ScriptFile((resources::ResourceManager*)owner, file->metadata, (const unsigned char*)buffer, bufferSize);
}

void handleShaders(void* owner, carton::File* file, const char* buffer, uint64_t bufferSize) {
	((resources::ResourceManager*)owner)->metadataToResource[file->metadata]
		= new resources::ShaderSource((resources::ResourceManager*)owner, file->metadata, (const unsigned char*)buffer, bufferSize);
}

void handleDKSHShaders(void* owner, carton::File* file, const char* buffer, uint64_t bufferSize) {
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

void handleMaps(void* owner, carton::File* file, const char* buffer, uint64_t bufferSize) {
	((resources::ResourceManager*)owner)->metadataToResource[file->metadata]
		= new resources::MapSource((resources::ResourceManager*)owner, file->metadata, (const unsigned char*)buffer, bufferSize);
}

void handleSounds(void* owner, carton::File* file, const char* buffer, uint64_t bufferSize) {
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
	#ifdef __linux__
	this->inotify = inotify_init();
	if(this->inotify < 0) {
		console::error("could not init inotify\n");
	}
	
	fcntl(this->inotify, F_SETFL, fcntl(this->inotify, F_GETFL, 0) | O_NONBLOCK);

	this->watch = inotify_add_watch(this->inotify, fileName.c_str(), IN_CREATE | IN_DELETE | IN_MODIFY);

	if(this->watch < 0) {
		console::error("could not add inotify watch\n");
	}

	console::print("watching %s for changes\n", fileName.c_str());
	#endif
	
	this->fileName = fileName;
	this->reload();
}

resources::ResourceManager::~ResourceManager() {
	#ifdef __linux__
	if(this->watch >= 0) {
		inotify_rm_watch(this->inotify, this->watch);
	}
	
	if(this->inotify >= 0) {
		close(this->inotify);
	}
	#endif
}

void resources::ResourceManager::tick() {
	#ifdef __linux__
	inotify_event event;
	int result = -100;
	while((result = read(this->inotify, (void*)&event, sizeof(event))) > 0) {
		char* name = nullptr;
		if(event.len > 0) {
			name = new char[event.len];
			read(this->inotify, name, event.len);
		}

		this->lastEvent = getMicrosecondsNow();
		this->hashed = false;
	}

	if(result < 0 && errno != EAGAIN) {
		console::error("failed to read from inotify\n");
	}

	if(getMicrosecondsNow() - this->lastEvent > 100000 && !this->hashed) {
		md5hash(this->fileName, this->hash.hash);
		this->hashed = true;
	}

	if(!(this->carton->hash == this->hash)) {
		console::print("----------------------------------------------------------------------\n");

		string hash = "";
		for(unsigned int i = 0; i < MD5_DIGEST_LENGTH; i++) {
			hash += fmt::format("{:x}", this->hash.hash[i]);
		}

		console::print("reloading %s (%s) after detecting changes...\n", this->fileName.c_str(), hash.c_str());
		this->reload();

		console::print("successfully reloaded\n");
		console::print("----------------------------------------------------------------------\n");
	}
	#endif
}

void resources::ResourceManager::reload() {
	bool reload = false;
	if(this->carton != nullptr) {
		delete this->carton;
		reload = true;
	}
	
	this->carton = new carton::Carton();
	this->carton->read(this->fileName);

	if(reload) {
		for(ResourceObject* object: this->objects) {
			carton::FileBuffer buffer = this->carton->readFileToBuffer(object->fileName);
			carton::Metadata* metadata = this->carton->database.get()->equals("fileName", object->fileName)->exec()[0];
			this->metadataToResource[metadata] = object;
			object->reload(metadata, buffer.buffer, buffer.size);
			free((void*)buffer.buffer);
		}
	}
	else {
		this->hash = this->carton->hash;
	}

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
	for(uint64_t i = 0; i < resources.head; i++) {
		if(resources[i]->getMetadata("stream") == "true") {
			console::error("Cannot load streamed file '%s' into memory\n", resources[i]->getMetadata("fileName").c_str());
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
	for(uint64_t i = 0; i < resources.head; i++) {
		output[i] = this->metadataToResource[resources[i]];
	}
	return output;
}

void es::defineResourceManager() {
	esRegisterFunction(engine->eggscript, ES_ENTRY_EMPTY, hotReload, "hotReload", 0, nullptr);
}

esEntryPtr es::hotReload(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	engine->manager->reload();
	return nullptr;
}
