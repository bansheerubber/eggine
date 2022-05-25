#include "spriteSheet.h"

#include "../engine/engine.h"
#include "resourceManager.h"

resources::SpriteSheet::SpriteSheet(
	ResourceManager &manager,
	carton::Metadata* metadata,
	const unsigned char* buffer,
	uint64_t bufferSize
) : ResourceObject(manager, metadata) {
	this->texture = new render::Texture(&engine->renderWindow);
	this->texture->setFilters(render::TEXTURE_FILTER_NEAREST, render::TEXTURE_FILTER_NEAREST);
	this->texture->setWrap(render::TEXTURE_WRAP_CLAMP_TO_BORDER, render::TEXTURE_WRAP_CLAMP_TO_BORDER);
	this->texture->loadPNG(buffer, bufferSize);
	
	this->spriteSheetWidth = stoi(metadata->getMetadata("width"));
	this->spriteSheetAmount = stoi(metadata->getMetadata("amount"));

	// allocate memory
	this->spriteInfo.allocate(this->spriteSheetAmount);

	for(uint64_t i = 0; i < this->spriteSheetAmount; i++) {
		std::string info = metadata->getMetadata("sprite" + std::to_string(i));
		this->spriteInfo[i].wall = NO_WALL;
		this->spriteInfo[i].index = (unsigned int)i;
		if(info.size()) {
			do {
				uint64_t position = info.find(' ');
				position = position != std::string::npos
					? position
					: info.size();
				std::string word = info.substr(0, position);
				info = info.substr(position >= info.size() ? info.size() : position + 1, info.size());

				// figure out what to do with info
				if(word == "wallNorth") {
					this->spriteInfo[i].wall = WALL_NORTH;
				}
				else if(word == "wallEast") {
					this->spriteInfo[i].wall = WALL_EAST;
				}
				else if(word == "wallSouth") {
					this->spriteInfo[i].wall = WALL_SOUTH;
				}
				else if(word == "wallWest") {
					this->spriteInfo[i].wall = WALL_WEST;
				}
				else if(word == "cornerNorthEast") {
					this->spriteInfo[i].wall = CORNER_NORTH_EAST;
				}
				else if(word == "cornerSouthEast") {
					this->spriteInfo[i].wall = CORNER_SOUTH_EAST;
				}
				else if(word == "cornerSouthWest") {
					this->spriteInfo[i].wall = CORNER_SOUTH_WEST;
				}
				else if(word == "cornerNorthWest") {
					this->spriteInfo[i].wall = CORNER_NORTH_WEST;
				}

				if(word == "north") {
					this->spriteInfo[i].facing = FACING_NORTH;
				}
				else if(word == "east") {
					this->spriteInfo[i].facing = FACING_EAST;
				}
				else if(word == "south") {
					this->spriteInfo[i].facing = FACING_SOUTH;
				}
				else if(word == "west") {
					this->spriteInfo[i].facing = FACING_WEST;
				}
				else if(word == "northeast") {
					this->spriteInfo[i].facing = FACING_NORTH_EAST;
				}
				else if(word == "southeast") {
					this->spriteInfo[i].facing = FACING_SOUTH_EAST;
				}
				else if(word == "southwest") {
					this->spriteInfo[i].facing = FACING_SOUTH_WEST;
				}
				else if(word == "northwest") {
					this->spriteInfo[i].facing = FACING_NORTH_WEST;
				}

				// handle spritesheet roots/facing
				uint64_t rootPos = 0;
				if((rootPos = word.find("root")) != std::string::npos) {
					unsigned int root = stoi(word.substr(0, rootPos));
					if(this->spriteFacingInfo.find(root) == this->spriteFacingInfo.end()) {
						this->spriteFacingInfo[root] = new SpriteFacingInfo {
							facings: tsl::robin_map<SpriteFacing, unsigned int>(),
							root: root,
						};
					}

					this->spriteInfo[i].facingsMap = this->spriteFacingInfo[root];
					this->spriteInfo[i].facingsMap->facings[this->spriteInfo[i].facing] = i;
				}
			}
			while(info.size() > 0);
		}
		this->spriteInfo.pushed();
	}
}

resources::SpriteSheetInfo resources::SpriteSheet::getSpriteInfo(uint64_t index) {
	if(index < this->spriteInfo.head) {
		return this->spriteInfo[index];
	}
	return SpriteSheetInfo();
}

bool resources::SpriteSheet::drawOntopOfOverlap(uint64_t spriteIndex) {
	switch(this->spriteInfo[spriteIndex].wall) {
		case WALL_NORTH: {
			return false;
		}

		case WALL_EAST: {
			return false;
		}

		case WALL_SOUTH: {
			return true;
		}

		case WALL_WEST: {
			return true;
		}

		case CORNER_NORTH_EAST: {
			return false;
		}
		
		case CORNER_SOUTH_EAST: {
			return true;
		}

		case CORNER_SOUTH_WEST: {
			return true;
		}

		case CORNER_NORTH_WEST: {
			return true;
		}
		
		default: {
			return false;
		}
	}
}
