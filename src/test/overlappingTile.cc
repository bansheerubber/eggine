#include "overlappingTile.h"

#include "chunk.h"
#include "chunkContainer.h"
#include "../engine/engine.h"
#include "tileMath.h"

OverlappingTile::OverlappingTile(ChunkContainer* container) : RenderObject(false) {
	this->container = container;	

	this->vertexAttributes = new render::VertexAttributes(&engine->renderWindow);

	// load vertices
	{
		this->vertexAttributes->addVertexAttribute(Chunk::VertexBuffers[1], 0, 2, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec2), 0);
	}

	// load uvs
	{
		this->vertexAttributes->addVertexAttribute(Chunk::VertexBuffers[2], 1, 2, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec2), 0);
	}

	// load offsets
	{
		this->vertexBuffers[0] = new render::VertexBuffer(&engine->renderWindow);
		this->vertexBuffers[0]->setDynamicDraw(true);
		this->vertexBuffers[0]->setData(&this->screenSpacePosition[0], sizeof(glm::vec2), alignof(glm::vec2));

		this->vertexAttributes->addVertexAttribute(this->vertexBuffers[0], 2, 2, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec2), 1);
	}

	// load texture indices
	{
		this->vertexBuffers[1] = new render::VertexBuffer(&engine->renderWindow);
		this->vertexBuffers[1]->setDynamicDraw(true);
		this->vertexBuffers[1]->setData(&this->textureIndex, sizeof(int), alignof(int));

		this->vertexAttributes->addVertexAttribute(this->vertexBuffers[1], 3, 1, render::VERTEX_ATTRIB_INT, 0, sizeof(int), 1);
	}
}

OverlappingTile::~OverlappingTile() {
	if(this->chunk != nullptr) {
		this->chunk->removeOverlappingTile(this);
	}
}

OverlappingTile* OverlappingTile::setPosition(glm::uvec3 position) {
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

	this->vertexBuffers[0]->setData(&this->screenSpacePosition[0], sizeof(glm::vec2), alignof(glm::vec2));

	this->chunk->updateOverlappingTile(this);
	return this;
}

glm::uvec3 OverlappingTile::getPosition() {
	return this->position;
}

OverlappingTile* OverlappingTile::setTexture(unsigned int index) {
	this->textureIndex = index;
	this->vertexBuffers[1]->setData(&this->textureIndex, sizeof(int), alignof(int));
	return this;
}

OverlappingTile* OverlappingTile::setColor(glm::vec4 color) {
	this->color = color;
	return this;
}

glm::vec4 OverlappingTile::getColor() {
	return this->color;
}

OverlappingTile* OverlappingTile::setZIndex(unsigned int zIndex) {
	this->zIndex = zIndex;
	return this;
}

unsigned int OverlappingTile::getZIndex() {
	return this->zIndex;	
}

ChunkContainer* OverlappingTile::getContainer() {
	return this->container;
}

void OverlappingTile::render(double deltaTime, RenderContext &context) {
	struct FragmentBlock {
		glm::vec4 color;
	} fb;
	fb.color = this->color;
	
	this->vertexAttributes->bind();
	ChunkContainer::Program->bindUniform("fragmentBlock", &fb, sizeof(fb));
	engine->renderWindow.draw(render::PRIMITIVE_TRIANGLE_STRIP, 0, 4, 0, 1);
}
