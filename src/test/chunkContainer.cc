#include <glad/gl.h>
#include "chunkContainer.h"

#include "../basic/camera.h"
#include "chunk.h"
#include "tileMath.h"

GLuint ChunkContainer::Shaders[2] = {GL_INVALID_INDEX, GL_INVALID_INDEX};
GLuint ChunkContainer::Uniforms[3] = {GL_INVALID_INDEX, GL_INVALID_INDEX, GL_INVALID_INDEX};
GLuint ChunkContainer::ShaderProgram = GL_INVALID_INDEX;

void initChunk(class ChunkContainer* container, class Chunk** chunk) {
	*chunk = nullptr;
}

ChunkContainer::ChunkContainer() {
	RenderObject::CompileShader(
		GL_VERTEX_SHADER,
		&ChunkContainer::Shaders[0],
		#include "shaders/test.vert"
	);

	RenderObject::CompileShader(
		GL_FRAGMENT_SHADER,
		&ChunkContainer::Shaders[1],
		#include "shaders/test.frag"
	);

	if(RenderObject::LinkProgram(&ChunkContainer::ShaderProgram, ChunkContainer::Shaders, 2)) {
		ChunkContainer::Uniforms[0] = glGetUniformLocation(ChunkContainer::ShaderProgram, "projection");
		ChunkContainer::Uniforms[1] = glGetUniformLocation(ChunkContainer::ShaderProgram, "spriteTexture");
		ChunkContainer::Uniforms[2] = glGetUniformLocation(ChunkContainer::ShaderProgram, "chunkScreenSpace");
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
	glUseProgram(ChunkContainer::ShaderProgram);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Chunk::Texture);
	glUniform1i(ChunkContainer::Uniforms[1], 0); // bind texture

	glUniformMatrix4fv(ChunkContainer::Uniforms[0], 1, false, &context.camera->projectionMatrix[0][0]);

	for(size_t i = 0; i < this->renderOrder.head; i++) {
		if(this->renderOrder[i] != nullptr) {
			this->renderOrder[i]->render(deltaTime, context);
		}
	}
}
