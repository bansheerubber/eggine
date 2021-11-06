#pragma once

#include "../util/dynamicArray.h"
#include "resourceObject.h"
#include "../renderer/texture.h"

namespace resources {
	enum SpriteSheetWall {
		NO_WALL						= 0b00000000,
		WALL_NORTH 				= 0b00000001,
		WALL_EAST 				= 0b00000010,
		WALL_SOUTH 				= 0b00000100,
		WALL_WEST 				= 0b00001000,
		CORNER_NORTH_EAST	= 0b00010000,
		CORNER_SOUTH_EAST	= 0b00100000,
		CORNER_SOUTH_WEST	= 0b01000000,
		CORNER_NORTH_WEST	= 0b10000000,
	};
	
	struct SpriteSheetInfo {
		SpriteSheetWall wall;
	};

	void initSpriteSheetInfo(class SpriteSheet* owner, SpriteSheetInfo* wall);
	
	class SpriteSheet: public ResourceObject {
		public:
			SpriteSheet(class ResourceManager* manager, carton::Metadata* metadata, const unsigned char* buffer, size_t bufferSize);

			render::Texture* texture = nullptr;

			// whether or not the tile should be drawn ontop of a overlapping tile
			bool drawOntopOfOverlap(size_t spriteIndex);
			SpriteSheetInfo getSpriteInfo(size_t index);
		
		protected:			
			unsigned int spriteSheetWidth = 0; // number of sprites per row
			unsigned int spriteSheetAmount = 0; // number of sprites in PNG
			
			DynamicArray<SpriteSheetInfo, SpriteSheet> spriteInfo = DynamicArray<SpriteSheetInfo, SpriteSheet>(this, 8, initSpriteSheetInfo, nullptr);
	};
};
