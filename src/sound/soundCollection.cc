#include "soundCollection.h"

#include <filesystem>
#include <regex>
#include <sstream>

#include "../engine/console.h"
#include "../engine/engine.h"
#include "sound.h"
#include "../util/trim.h"

sound::SoundCollection::SoundCollection(
	resources::ResourceManager &manager,
	carton::Metadata* metadata,
	const unsigned char* buffer,
	uint64_t bufferSize
) : ResourceObject(manager, metadata) {
	std::string fileBase = std::filesystem::path(metadata->getMetadata("fileName")).parent_path().string();
	std::replace(fileBase.begin(), fileBase.end(), '\\', '/');
	
	std::istringstream stream(std::string((const char*)buffer, bufferSize));
	for(std::string line; std::getline(stream, line);) {
		if(trim(line).length() == 0) {
			continue;
		}

		uint64_t equalsPosition = line.find("=");
		std::string key = trim(line.substr(0, equalsPosition));
		std::string value = trim(line.substr(equalsPosition + 1, line.length() - equalsPosition));

		if(key == "soundName") {
			this->name = value;
		}
		else if(key == "pitchMin") {
			this->pitchMin = stof(value);
		}
		else if(key == "pitchMax") {
			this->pitchMax = stof(value);
		}
		else if(std::regex_match(key, std::regex("sound[0-9]+"))) {
			std::string fileName = fileBase + "/" + value;
			fileName = std::filesystem::path(fileName).lexically_normal().string();
			std::replace(fileName.begin(), fileName.end(), '\\', '/');
			if(engine->soundEngine.fileToSound.find(fileName) != engine->soundEngine.fileToSound.end()) {
				this->sounds.push_back(engine->soundEngine.fileToSound[fileName]);
			}
			else {
				console::error("Cannot create sound collection from '%s' using missing sound file '%s'\n", metadata->getMetadata("fileName").c_str(), fileName.c_str());
				exit(1);
			}
		}
	}

	if(this->name == "") {
		console::error("Cannot create sound collection from '%s' with no name\n", metadata->getMetadata("fileName").c_str());
		exit(1);
	}

	engine->soundEngine.addCollection(this);
}

void sound::SoundCollection::play() {
	SoundSourceProperties properties;
	properties.pitch = ((double)rand() / (double)RAND_MAX) * (this->pitchMax - this->pitchMin) + this->pitchMin;
	this->sounds[rand() % this->sounds.size()]->play(properties);
}
