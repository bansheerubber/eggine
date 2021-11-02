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

class Tile;

class Chunk : public InstancedRenderObjectContainer<Tile> {
	friend class ChunkContainer;
	friend class OverlappingTile;
	
	public:
		Chunk(class ChunkContainer* container);
		~Chunk();

		// ## game_object_definitions Chunk

		int* textureIndices = nullptr;
		unsigned int height = 5;

		void render(double deltaTime, RenderContext &context) {};
		void renderChunk(double deltaTime, RenderContext &context);

		void setPosition(glm::uvec2 position);
		glm::uvec2& getPosition();

		class Layer* getLayer(unsigned int z);

		void setTileTexture(glm::uvec2 position, unsigned int spritesheetIndex);

		static constexpr unsigned int Size = 25;
	
	protected:
		class ChunkContainer* container;

		tsl::robin_set<class OverlappingTile*> overlappingTiles;
		tsl::robin_map<unsigned int, class Layer*> layers;
		unsigned int maxLayer = 0;

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

		static glm::vec2 OffsetsSource[];
		static render::VertexBuffer* Offsets;
};
