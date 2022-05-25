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
		FACING_NORTH_EAST,
		FACING_SOUTH_EAST,
		FACING_SOUTH_WEST,
		FACING_NORTH_WEST,
	};

	struct SpriteSheetInfo {
		struct SpriteFacingInfo* facingsMap;
		SpriteFacing facing;
		unsigned int index;
		SpriteSheetWall wall;

		SpriteSheetInfo() {
			this->facingsMap = nullptr;
			this->facing = FACING_INVALID;
			this->index = 0;
			this->wall = NO_WALL;
		}
	};

	struct SpriteFacingInfo { // records all sprite facings for a particular root sprite
		tsl::robin_map<SpriteFacing, unsigned int> facings;
		unsigned int root;

		unsigned int rotateFacing(SpriteFacing facing, tilemath::Rotation oldRotation, tilemath::Rotation newRotation) {
			if(facing < FACING_NORTH_EAST) {
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
			else {
				int delta = (int)newRotation - (int)oldRotation;
				int newFacing = ((int)facing - 5) - delta;
				if(newFacing < 0) {
					newFacing += 4;
				}
				else {
					newFacing = newFacing % 4;
				}
				return this->facings[(SpriteFacing)(newFacing + 5)];
			}
		}

		unsigned int getFacing(SpriteFacing facing, tilemath::Rotation rotation) {
			if(facing < FACING_NORTH_EAST) {
				int newFacing = (facing - 1) - rotation;
				if(newFacing < 0) {
					newFacing += 4;
				}
				else {
					newFacing = newFacing % 4;
				}
				return this->facings[(SpriteFacing)(newFacing + 1)];
			}
			else {
				int newFacing = (facing - 5) - rotation;
				if(newFacing < 0) {
					newFacing += 4;
				}
				else {
					newFacing = newFacing % 4;
				}
				return this->facings[(SpriteFacing)(newFacing + 5)];
			}
		}
	};

	class SpriteSheet: public ResourceObject {
		public:
			SpriteSheet(ResourceManager &manager, carton::Metadata* metadata, const unsigned char* buffer, uint64_t bufferSize);

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
			
			DynamicArray<SpriteSheetInfo> spriteInfo = DynamicArray<SpriteSheetInfo>(8);
			tsl::robin_map<unsigned int, SpriteFacingInfo*> spriteFacingInfo; // root sprite to facing object
	};
};
