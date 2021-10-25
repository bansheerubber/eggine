#include "spriteSheet.h"

#include "../engine/engine.h"
#include "resourceManager.h"

void resources::initSpriteSheetInfo(SpriteSheet* owner, SpriteSheetInfo* wall) {
	*wall = {
		wall: NO_WALL,
	};
}

resources::SpriteSheet::SpriteSheet(
	ResourceManager* manager,
	carton::Metadata* metadata,
	const unsigned char* buffer,
	size_t bufferSize
) : ResourceObject(manager, metadata) {
	this->texture = new render::Texture(&engine->renderWindow);
	this->texture->setFilters(render::TEXTURE_FILTER_NEAREST, render::TEXTURE_FILTER_NEAREST);
	this->texture->setWrap(render::TEXTURE_WRAP_CLAMP_TO_BORDER, render::TEXTURE_WRAP_CLAMP_TO_BORDER);
	this->texture->loadPNG(buffer, bufferSize);
	
	this->spriteSheetWidth = stoi(metadata->getMetadata("width"));
	this->spriteSheetAmount = stoi(metadata->getMetadata("amount"));

	// allocate memory
	this->spriteInfo.allocate(this->spriteSheetAmount);

	for(size_t i = 0; i < this->spriteSheetAmount; i++) {
		string info = metadata->getMetadata("sprite" + to_string(i));
		if(info.size()) {
			do {
				size_t position = info.find(' ');
				position = position != string::npos
					? position
					: info.size();
				string word = info.substr(0, position);
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
			}
			while(info.size() > 0);
		}
	}
}

bool resources::SpriteSheet::drawOntopOfOverlap(size_t spriteIndex) {
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
