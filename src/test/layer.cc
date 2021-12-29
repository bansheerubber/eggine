#include "layer.h"

#include "chunk.h"
#include "chunkContainer.h"
#include "../engine/engine.h"
#include "overlappingTile.h"
#include "tileMath.h"

int layerTilesCompare(class OverlappingTile** a, class OverlappingTile** b) {
	unsigned int indexA = tilemath::coordinateToIndex((*a)->getPosition(), Chunk::Size, engine->chunkContainer->getRotation());
	unsigned int indexB = tilemath::coordinateToIndex((*b)->getPosition(), Chunk::Size, engine->chunkContainer->getRotation());
	if(indexA > indexB) {
		return 1;
	}
	else if(indexA < indexB) {
		return -1;
	}
	else if((*a)->getZIndex() > (*b)->getZIndex()) {
		return 1;
	}
	else if((*a)->getZIndex() < (*b)->getZIndex()) {
		return -1;
	}
	else {
		return 0;
	}
}

Layer::Layer(Chunk* chunk) {
	this->chunk = chunk;

	this->buffers[0] = new render::VertexBuffer(&engine->renderWindow);
	this->buffers[1] = new render::VertexBuffer(&engine->renderWindow);
	this->buffers[2] = new render::VertexBuffer(&engine->renderWindow);

	this->attributes = new render::VertexAttributes(&engine->renderWindow);

	// load vertices
	{
		this->attributes->addVertexAttribute(ChunkContainer::Vertices, 0, 2, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec2), 0);
	}

	// load uvs
	{
		this->attributes->addVertexAttribute(ChunkContainer::UVs, 1, 2, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec2), 0);
	}

	// load offsets
	{
		this->buffers[0]->setDynamicDraw(true);
		this->buffers[0]->setData(nullptr, 0, sizeof(glm::vec2));
		this->attributes->addVertexAttribute(this->buffers[0], 2, 2, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec2), 1);
	}

	// load texture indices
	{
		this->buffers[1]->setDynamicDraw(true);
		this->buffers[1]->setData(nullptr, 0, sizeof(int));
		this->attributes->addVertexAttribute(this->buffers[1], 3, 1, render::VERTEX_ATTRIB_INT, 0, sizeof(int), 1);
	}

	// load color indices
	{
		this->buffers[2]->setDynamicDraw(true);
		this->buffers[2]->setData(nullptr, 0, sizeof(glm::vec4));
		this->attributes->addVertexAttribute(this->buffers[2], 4, 4, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec4), 1);
	}
}

void Layer::rebuildBuffers() {
	this->offsets.allocate(this->tiles.array.head);
	this->textureIndices.allocate(this->tiles.array.head);
	this->colors.allocate(this->tiles.array.head);
	
	// go through sorted list and build the texture/offset buffers
	for(uint64_t i = 0; i < this->tiles.array.head; i++) {
		OverlappingTile* tile = this->tiles.array[i];
		this->offsets[i] = tile->screenSpacePosition;
		this->textureIndices[i] = tile->getTexture();
		this->colors[i] = tile->getColor();
	}

	this->buffers[0]->setData(&this->offsets[0], sizeof(glm::vec2) * this->tiles.array.head, sizeof(glm::vec2));
	this->buffers[1]->setData(&this->textureIndices[0], sizeof(int) * this->tiles.array.head, sizeof(int));
	this->buffers[2]->setData(&this->colors[0], sizeof(glm::vec4) * this->tiles.array.head, sizeof(glm::vec4));
}

void Layer::add(OverlappingTile* tile) {
	this->tiles.insert(tile);
	this->needsUpdate = true;
}

void Layer::update(OverlappingTile* tile) {
	this->needsUpdate = true;
}

void Layer::remove(OverlappingTile* tile) {
	this->tiles.remove(tile);
	this->needsUpdate = true;
}

void Layer::updateRotation(tilemath::Rotation oldRotation, tilemath::Rotation newRotation) {
	this->tiles.sort();
	for(uint64_t i = 0; i < this->tiles.array.head; i++) {
		this->tiles.array[i]->updateRotation(oldRotation, newRotation);
	}
	this->rebuildBuffers();
}

void Layer::render(double deltaTime, RenderContext &context) {
	struct FragmentBlock {
		glm::vec4 color;
	} fb;
	fb.color = glm::vec4(1, 1, 1, 1);

	if(this->needsSort) {
		this->tiles.sort();
		this->needsSort = false;
	}

	if(this->needsUpdate) {
		this->rebuildBuffers();
		this->needsUpdate = false;
	}
	
	this->attributes->bind();
	engine->renderWindow.draw(render::PRIMITIVE_TRIANGLE_STRIP, 0, 4, 0, this->tiles.array.head);
}
