#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <tsl/robin_map.h>

#include "../engine/debug.h"
#include "../basic/instancedRenderObjectContainer.h"
#include "../basic/renderContext.h"
#include "../basic/renderObject.h"
#include "../util/sortedArray.h"
#include "../resources/spriteSheet.h"

class Tile;

struct OverlappingTileWrapper {
	unsigned int index;
	class OverlappingTile* tile;

	bool operator<(const OverlappingTileWrapper &other) {
		return this->index < other.index;
	}

	bool operator>(const OverlappingTileWrapper &other) {
		return this->index > other.index;
	}

	bool operator==(const OverlappingTileWrapper &other) {
		return this->tile == other.tile;
	}
};

int compareOverlappingTile(const void* a, const void* b);
void initOverlappingTileWrapper(class Chunk* chunk, OverlappingTileWrapper* tile);

class Chunk : public InstancedRenderObjectContainer<Tile> {
	friend class ChunkContainer;
	friend class OverlappingTile;
	
	public:
		Chunk();
		~Chunk();

		// ## game_object_definitions Chunk

		int* textureIndices = nullptr;
		unsigned int height = 5;

		void render(double deltaTime, RenderContext &context) {};
		void renderChunk(double deltaTime, RenderContext &context);

		void setPosition(glm::uvec2 position);
		glm::uvec2& getPosition();

		static constexpr unsigned int Size = 25;
		// static GLuint Texture;
	
	protected:
		SortedArray<OverlappingTileWrapper, Chunk> overlappingTiles = SortedArray<OverlappingTileWrapper, Chunk>(this, compareOverlappingTile, initOverlappingTileWrapper, nullptr);

		glm::uvec2 position = glm::uvec2(0, 0);
		glm::vec2 screenSpacePosition;
		
		render::VertexBuffer* vertexBuffer;
		render::VertexAttributes* vertexAttributes;
		class Line* debugLine = nullptr;
		double top = 0, right = 0, bottom = 0, left = 0;

		#ifdef EGGINE_DEBUG
		bool isCulled = false;
		int drawCalls = 0;
		#endif

		void addOverlappingTile(class OverlappingTile* tile);
		void updateOverlappingTile(class OverlappingTile* tile);
		void removeOverlappingTile(class OverlappingTile* tile);
		
		void buildDebugLines();
		void defineBounds();

		static glm::vec2 Offsets[];
		static render::VertexBuffer* VertexBuffers[];

		static constexpr glm::vec2 Vertices[8] = {
			glm::vec2(-0.501f,  1.001f),
			glm::vec2(-0.501f, -1.001f),
			glm::vec2(0.501f, 1.001f),
			glm::vec2(0.501f, -1.001f)
		};

		static constexpr glm::vec2 UVs[8] = {
			glm::vec2(0.0f, 0.0f),
			glm::vec2(0.0f, 1.0),
			glm::vec2(1.0, 0.0f),
			glm::vec2(1.0, 1.0)
		};
};
