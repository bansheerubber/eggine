#pragma once

#include <vector>

#include "../util/dynamicArray.h"
#include "../basic/renderObject.h"

using namespace std;

void initChunk(class ChunkContainer* container, class Chunk** chunk);

class ChunkContainer : public RenderObject {
	friend class Engine;
	
	public:
		ChunkContainer();

		// ## game_object_definitions ChunkContainer

		void addOverlappingTile(class OverlappingTile* tile);
		void removeOverlappingTile(class OverlappingTile* tile);
		void setOverlappingTileChunk(OverlappingTile* tile);
		
		void addChunk(class Chunk* container);
		void buildRenderOrder();

		void render(double deltaTime, RenderContext &context);

		static GLuint Shaders[];
		static GLuint Uniforms[];
		static GLuint ShaderProgram;

		vector<class Chunk*> chunks;

		unsigned int size = 0;
	
	private:
		vector<class OverlappingTile*> overlappingTiles;
		DynamicArray<class Chunk*, ChunkContainer> renderOrder = DynamicArray<class Chunk*, ChunkContainer>(this, 8, initChunk, nullptr);
};
