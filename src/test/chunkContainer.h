#pragma once

#include <vector>

#include "chunk.h"
#include "../util/dynamicArray.h"
#include "../basic/renderObject.h"
#include "../resources/spriteSheet.h"

using namespace std;

void initChunk(class ChunkContainer* container, class Chunk* chunk);

class ChunkContainer : public RenderObject {
	friend class Engine;
	
	public:
		ChunkContainer();
		~ChunkContainer();

		// ## game_object_definitions ChunkContainer

		void addOverlappingTile(class OverlappingTile* tile);
		void removeOverlappingTile(class OverlappingTile* tile);
		void setOverlappingTileChunk(OverlappingTile* tile);
		
		void addChunk(glm::uvec2 position);
		Chunk& getChunk(size_t index);
		size_t getChunkCount();

		void render(double deltaTime, RenderContext &context);

		static class Shader* Program;
		static resources::SpriteSheet* Image;

		unsigned int size = 0;
	
	private:
		DynamicArray<Chunk, ChunkContainer> renderOrder = DynamicArray<Chunk, ChunkContainer>(this, 8, initChunk, nullptr);
};
