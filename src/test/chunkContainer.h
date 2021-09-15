#pragma once

#include <vector>

#include "../util/dynamicArray.h"
#include "../basic/renderObject.h"

using namespace std;

void initChunk(class ChunkContainer* container, class Chunk** chunk);

class ChunkContainer: public RenderObject {
	public:
		ChunkContainer();
		
		void addChunk(class Chunk* container);
		void buildRenderOrder();

		void render(double deltaTime, RenderContext &context);
	
	private:
		GLuint shaders[2] = {GL_INVALID_INDEX, GL_INVALID_INDEX};
		GLuint uniforms[2];
		GLuint shaderProgram = GL_INVALID_INDEX;
		
		vector<class Chunk*> chunks;
		DynamicArray<class Chunk*, ChunkContainer> renderOrder = DynamicArray<class Chunk*, ChunkContainer>(this, 8, initChunk, nullptr);
};
