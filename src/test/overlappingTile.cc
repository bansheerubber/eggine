#include <glad/gl.h>
#include "overlappingTile.h"

#include "chunk.h"
#include "chunkContainer.h"
#include "tileMath.h"

OverlappingTile::OverlappingTile(ChunkContainer* container) : RenderObject(false) {
	this->container = container;	
	
	glGenBuffers(2, this->vertexBufferObjects);
	glGenVertexArrays(1, &this->vertexArrayObject);
	glBindVertexArray(this->vertexArrayObject);

	// load vertices
	{
		glBindBuffer(GL_ARRAY_BUFFER, Chunk::VertexBufferObjects[1]);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);
	}

	// load uvs
	{
		glBindBuffer(GL_ARRAY_BUFFER, Chunk::VertexBufferObjects[2]);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);
	}

	// load offsets
	{
		glBindBuffer(GL_ARRAY_BUFFER, this->vertexBufferObjects[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::lowp_vec2), NULL, GL_DYNAMIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::lowp_vec2), &this->screenSpacePosition[0]);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribDivisor(2, 1);
		glEnableVertexAttribArray(2);
	}

	// load texture indices
	{
		glBindBuffer(GL_ARRAY_BUFFER, this->vertexBufferObjects[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(int), &this->textureIndex, GL_STATIC_DRAW);

		glVertexAttribIPointer(3, 1, GL_INT, 0, 0);
		glVertexAttribDivisor(3, 1);
		glEnableVertexAttribArray(3);
	}

	glBindVertexArray(0); // turn off vertex array object
}

OverlappingTile::~OverlappingTile() {
	glDeleteBuffers(2, this->vertexBufferObjects);
	glDeleteVertexArrays(1, &this->vertexArrayObject);

	if(this->chunk != nullptr) {
		this->chunk->removeOverlappingTile(this);
	}
}

void OverlappingTile::setPosition(glm::uvec3 position) {
	this->position = position;

	glm::uvec2 chunkPosition(position.x / Chunk::Size, position.y / Chunk::Size);
	Chunk &chunk = this->container->getChunk(tilemath::coordinateToIndex(chunkPosition, this->container->size));

	if(this->chunk != &chunk) {
		if(this->chunk != nullptr) {
			this->chunk->removeOverlappingTile(this);
		}
		chunk.addOverlappingTile(this);
	}
	this->chunk = &chunk;

	glm::uvec3 relativePosition = this->position;
	relativePosition.x -= this->chunk->position.x * Chunk::Size; // we add the chunk position to the tile in the shader
	relativePosition.y -= this->chunk->position.y * Chunk::Size;
	this->screenSpacePosition = tilemath::tileToScreen(relativePosition);

	glBindBuffer(GL_ARRAY_BUFFER, this->vertexBufferObjects[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::lowp_vec2), NULL, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::lowp_vec2), &this->screenSpacePosition[0]);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	this->chunk->updateOverlappingTile(this);
}

glm::uvec3 OverlappingTile::getPosition() {
	return this->position;
}

void OverlappingTile::render(double deltaTime, RenderContext &context) {
	// TODO make smart shader binding

	glBindVertexArray(this->vertexArrayObject);
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 1);
}
