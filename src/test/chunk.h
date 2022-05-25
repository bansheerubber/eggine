#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <tsl/robin_map.h>
#include <tsl/robin_set.h>

#include "../engine/debug.h"
#include "../basic/instancedRenderObjectContainer.h"
#include "../basic/renderContext.h"
#include "../basic/renderObject.h"
#include "../util/sortedArray.h"
#include "../resources/spriteSheet.h"
#include "tileMath.h"

class Tile;

namespace std {
	template<>
	struct hash<pair<tilemath::Rotation, tilemath::Rotation>> {
		size_t operator()(pair<tilemath::Rotation, tilemath::Rotation> const& source) const noexcept {
			return source.first ^ (source.second + 0x9e3779b9 + (source.first << 6) + (source.first >> 2));
    }
	};

	template<>
	struct equal_to<pair<tilemath::Rotation, tilemath::Rotation>> {
		bool operator()(const pair<tilemath::Rotation, tilemath::Rotation>& x, const pair<tilemath::Rotation, tilemath::Rotation>& y) const {
			return x.first == y.first && x.second == y.second;
		}
	};
};

struct InterweavedTileWrapper {
	unsigned int index;
	class InterweavedTile* tile;

	InterweavedTileWrapper() {
		this->index = 0;
		this->tile = nullptr;
	}

	InterweavedTileWrapper(unsigned int index, class InterweavedTile* tile) {
		this->index = index;
		this->tile = tile;
	}

	bool operator<(const InterweavedTileWrapper &other) const;
	bool operator>(const InterweavedTileWrapper &other) const;
	bool operator==(const InterweavedTileWrapper &other) const;
};

namespace std {
	template<>
	struct greater<InterweavedTileWrapper> {
		bool operator()(const InterweavedTileWrapper &lhs, const InterweavedTileWrapper &rhs) const {
			return lhs > rhs;
		}
	};

	template<>
	struct less<InterweavedTileWrapper> {
		bool operator()(const InterweavedTileWrapper &lhs, const InterweavedTileWrapper &rhs) const {
			return lhs < rhs;
		}
	};
};

class Chunk : public InstancedRenderObjectContainer<Tile> {
	friend class ChunkContainer;
	
	public:
		Chunk(class ChunkContainer* container);
		~Chunk();

		// ## game_object_definitions Chunk

		void render(double deltaTime, RenderContext &context) {};
		void renderChunk(double deltaTime, RenderContext &context);
		void renderXRay(double deltaTime, RenderContext &context);

		void setPosition(glm::uvec2 position);
		glm::uvec2& getPosition();
		uint16_t getHeight();

		class Layer* getLayer(unsigned int z);

		void setTileTexture(glm::uvec3 position, unsigned int spritesheetIndex);
		int getTileTexture(glm::uvec3 position);

		void setTileTextureByIndex(uint64_t index, unsigned int spritesheetIndex);
		int getTileTextureByIndex(uint64_t index);

		void addOverlappingTile(class OverlappingTile* tile);
		void updateOverlappingTile(class OverlappingTile* tile);
		void removeOverlappingTile(class OverlappingTile* tile);

		void addInterweavedTile(class InterweavedTile* tile);
		void updateInterweavedTile(class InterweavedTile* tile);
		void removeInterweavedTile(class InterweavedTile* tile);

		static constexpr unsigned int Size = 25;
		static constexpr unsigned int MaxHeight = 15;
		static void BuildOffsets(tilemath::Rotation rotaton);
	
	protected:
		class ChunkContainer* container;

		SortedArray<InterweavedTileWrapper> interweavedTiles = SortedArray<InterweavedTileWrapper>();

		tsl::robin_set<class OverlappingTile*> overlappingTiles;
		tsl::robin_map<unsigned int, class Layer*> layers;
		unsigned int maxLayer = 0;

		glm::uvec2 position = glm::uvec2(0, 0);
		glm::vec3 screenSpacePosition = glm::vec3(0, 0, 0);
		uint16_t height = 5;
		
		render::VertexBuffer* vertexBuffer;
		render::VertexAttributes* vertexAttributes;
		class Line* debugLine = nullptr;
		double top = 0, right = 0, bottom = 0, left = 0;

		tilemath::Rotation oldRotation = tilemath::ROTATION_0_DEG;

		int* textureIndices = nullptr;

		#ifdef EGGINE_DEBUG
		bool isCulled = false;
		int drawCalls = 0;
		#endif

		uint64_t renderWithInterweavedTiles(uint64_t startInterweavedIndex, uint64_t startIndex, uint64_t amount, double deltaTime, RenderContext &context);

		void updateRotation(tilemath::Rotation rotation);
		
		void buildDebugLines();
		void defineBounds();

		static glm::vec3 OffsetsSource[];
		static render::VertexBuffer* Offsets;
		static tsl::robin_map<std::pair<tilemath::Rotation, tilemath::Rotation>, std::vector<int64_t>> Rotations;
};
