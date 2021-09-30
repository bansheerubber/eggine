#pragma once

#include "../util/dynamicArray.h"
#include "pngImage.h"

namespace resources {
	enum SpriteSheetWall {
		NO_WALL = 0,
		WALL_NORTH,
		WALL_EAST,
		WALL_SOUTH,
		WALL_WEST,
		CORNER_NORTH_EAST,
		CORNER_SOUTH_EAST,
		CORNER_SOUTH_WEST,
		CORNER_NORTH_WEST,
	};
	
	struct SpriteSheetInfo {
		SpriteSheetWall wall;
	};

	void initSpriteSheetInfo(class SpriteSheet* owner, SpriteSheetInfo* wall);
	
	class SpriteSheet: public PNGImage {
		public:
			SpriteSheet(class ResourceManager* manager, carton::Metadata* metadata, const unsigned char* buffer, size_t bufferSize);

			// whether or not the tile should be drawn ontop of a overlapping tile
			bool drawOntopOfOverlap(size_t spriteIndex);
		
		protected:
			unsigned int spriteSheetWidth = 0; // number of sprites per row
			unsigned int spriteSheetAmount = 0; // number of sprites in PNG
			
			DynamicArray<SpriteSheetInfo, SpriteSheet> spriteInfo = DynamicArray<SpriteSheetInfo, SpriteSheet>(this, 8, initSpriteSheetInfo, nullptr);
	};
};
