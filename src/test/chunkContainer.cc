#include <glad/gl.h>
#include "chunkContainer.h"

#include "../basic/camera.h"
#include "chunk.h"
#include "tileMath.h"

void initChunk(class ChunkContainer* container, class Chunk** chunk) {
	*chunk = nullptr;
}

ChunkContainer::ChunkContainer() {
	RenderObject::CompileShader(
		GL_VERTEX_SHADER,
		&this->shaders[0],
		#include "shaders/test.vert"
	);

	RenderObject::CompileShader(
		GL_FRAGMENT_SHADER,
		&this->shaders[1],
		#include "shaders/test.frag"
	);

	if(RenderObject::LinkProgram(&this->shaderProgram, this->shaders, 2)) {
		this->uniforms[0] = glGetUniformLocation(this->shaderProgram, "projection");
		this->uniforms[1] = glGetUniformLocation(this->shaderProgram, "spriteTexture");
	}
}

void ChunkContainer::addChunk(Chunk* chunk) {
	this->chunks.push_back(chunk);
}

void ChunkContainer::buildRenderOrder() {
	this->renderOrder.head = 0;

	// find the largest chunk extent
	int largestAxis = -1;
	for(auto chunk: this->chunks) {
		largestAxis = max(chunk->position.x, largestAxis);
		largestAxis = max(chunk->position.y, largestAxis);
	}

	largestAxis += 1;

	// allocate space
	for(size_t i = 0; i < largestAxis * largestAxis; i++) {
		this->renderOrder.pushed();
	}
	
	// rebuild render order
	for(auto chunk: this->chunks) {
		this->renderOrder[tilemath::coordinateToIndex(chunk->position, largestAxis)] = chunk;
	}
}

void ChunkContainer::render(double deltaTime, RenderContext &context) {
	glUseProgram(this->shaderProgram);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Chunk::Texture);
	glUniform1i(this->uniforms[1], 0); // bind texture

	glUniformMatrix4fv(this->uniforms[0], 1, false, &context.camera->projectionMatrix[0][0]);

	for(size_t i = 0; i < this->renderOrder.head; i++) {
		if(this->renderOrder[i] != nullptr) {
			this->renderOrder[i]->render(deltaTime, context);
		}
	}
}
