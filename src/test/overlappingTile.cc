#include "overlappingTile.h"

#include "chunk.h"
#include "chunkContainer.h"
#include "../engine/engine.h"
#include "tileMath.h"

OverlappingTile::OverlappingTile(ChunkContainer* container) : GameObject() {
	this->container = container;	
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

	glm::uvec3 relativePosition = this->position;
	relativePosition.x -= chunk.position.x * Chunk::Size; // we add the chunk position to the tile in the shader
	relativePosition.y -= chunk.position.y * Chunk::Size;
	this->screenSpacePosition = tilemath::tileToScreen(relativePosition);

	bool initialized = false;
	if(this->chunk != &chunk) {
		if(this->chunk != nullptr) {
			this->chunk->removeOverlappingTile(this);
		}
		chunk.addOverlappingTile(this);
		initialized = true;
	}
	this->chunk = &chunk;

	if(!initialized) {
		this->chunk->updateOverlappingTile(this);
	}

	this->layer = this->chunk->getLayer(position.z);

	return this;
}

glm::uvec3 OverlappingTile::getPosition() {
	return this->position;
}

Layer* OverlappingTile::getLayer() {
	return this->layer;
}

OverlappingTile* OverlappingTile::setTexture(unsigned int index) {
	this->textureIndex = index;
	if(this->chunk != nullptr) {
		this->chunk->updateOverlappingTile(this);
	}
	return this;
}

int OverlappingTile::getTexture() {
	return this->textureIndex;
}

OverlappingTile* OverlappingTile::setColor(glm::vec4 color) {
	this->color = color;
	if(this->chunk != nullptr) {
		this->chunk->updateOverlappingTile(this);
	}
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
