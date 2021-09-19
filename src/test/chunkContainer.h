#pragma once

#include <vector>

#include "../util/dynamicArray.h"
#include "../basic/renderObject.h"

using namespace std;

void initChunk(class ChunkContainer* container, class Chunk** chunk);

class ChunkContainer: public RenderObject {
	friend class Engine;
	
	public:
		ChunkContainer();

		// ## game_object_definitions ChunkContainer
		
		void addChunk(class Chunk* container);
		void buildRenderOrder();

		void render(double deltaTime, RenderContext &context);

		static GLuint Shaders[];
		static GLuint Uniforms[];
		static GLuint ShaderProgram;
	
	private:
		vector<class Chunk*> chunks;
		DynamicArray<class Chunk*, ChunkContainer> renderOrder = DynamicArray<class Chunk*, ChunkContainer>(this, 8, initChunk, nullptr);
};
