#pragma once

#include <tsl/robin_map.h>

#include "../engine/console.h"
#include "../util/dynamicArray.h"
#include "resourceObject.h"
#include "../renderer/texture.h"
#include "../test/tileMath.h"

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

	enum SpriteFacing {
		FACING_INVALID = 0,
		FACING_NORTH,
		FACING_EAST,
		FACING_SOUTH,
		FACING_WEST,
	};

	struct SpriteSheetInfo {
		struct SpriteFacingInfo* facingsMap;
		SpriteFacing facing;
		unsigned int index;
		SpriteSheetWall wall;
	};

	struct SpriteFacingInfo { // records all sprite facings for a particular root sprite
		tsl::robin_map<SpriteFacing, unsigned int> facings;
		unsigned int root;

		unsigned int rotateFacing(SpriteFacing facing, tilemath::Rotation oldRotation, tilemath::Rotation newRotation) {
			int delta = (int)newRotation - (int)oldRotation;
			int newFacing = ((int)facing - 1) - delta;
			if(newFacing < 0) {
				newFacing += 4;
			}
			else {
				newFacing = newFacing % 4;
			}
			return this->facings[(SpriteFacing)(newFacing + 1)];
		}
	};

	void initSpriteSheetInfo(class SpriteSheet* owner, SpriteSheetInfo* wall);
	
	class SpriteSheet: public ResourceObject {
		public:
			SpriteSheet(class ResourceManager* manager, carton::Metadata* metadata, const unsigned char* buffer, uint64_t bufferSize);

			render::Texture* texture = nullptr;

			// whether or not the tile should be drawn ontop of a overlapping tile
			bool drawOntopOfOverlap(uint64_t spriteIndex);
			SpriteSheetInfo getSpriteInfo(uint64_t index);

			void reload(carton::Metadata* metadata, const unsigned char* buffer, uint64_t bufferSize) {
				ResourceObject::reload(metadata, buffer, bufferSize);
				console::warning("spritesheet reload not implemented\n");
			}

			unsigned int getBytesUsed() {
				return 0;
			}
		
		protected:			
			unsigned int spriteSheetWidth = 0; // number of sprites per row
			unsigned int spriteSheetAmount = 0; // number of sprites in PNG
			
			DynamicArray<SpriteSheetInfo, SpriteSheet> spriteInfo = DynamicArray<SpriteSheetInfo, SpriteSheet>(this, 8, initSpriteSheetInfo, nullptr);
			tsl::robin_map<unsigned int, SpriteFacingInfo*> spriteFacingInfo; // root sprite to facing object
	};
};
