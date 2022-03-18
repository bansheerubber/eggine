#include "layer.h"

#include "chunk.h"
#include "chunkContainer.h"
#include "../engine/engine.h"
#include "overlappingTile.h"
#include "tileMath.h"

DynamicArray<glm::vec3> Layer::Offsets = DynamicArray<glm::vec3>(4);
DynamicArray<int> Layer::TextureIndices = DynamicArray<int>(4);
DynamicArray<glm::vec4> Layer::Colors = DynamicArray<glm::vec4>(4);

DynamicArray<glm::vec3> Layer::OccludedOffsets = DynamicArray<glm::vec3>(4);
DynamicArray<int> Layer::OccludedTextureIndices = DynamicArray<int>(4);
DynamicArray<glm::vec4> Layer::OccludedColors = DynamicArray<glm::vec4>(4);
DynamicArray<int> Layer::OccludedEnabled = DynamicArray<int>(4);

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
		this->buffers[0]->setData(nullptr, 0, sizeof(glm::vec3));
		this->attributes->addVertexAttribute(this->buffers[0], 2, 3, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec3), 1);
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

	// load occluded
	{
		this->attributes->addVertexAttribute(ChunkContainer::Occluded, 5, 1, render::VERTEX_ATTRIB_INT, 0, sizeof(int), 0);
	}

	// handle occluded tiles
	this->occludedBuffers[0] = new render::VertexBuffer(&engine->renderWindow);
	this->occludedBuffers[1] = new render::VertexBuffer(&engine->renderWindow);
	this->occludedBuffers[2] = new render::VertexBuffer(&engine->renderWindow);
	this->occludedBuffers[3] = new render::VertexBuffer(&engine->renderWindow);

	this->occludedAttributes = new render::VertexAttributes(&engine->renderWindow);

	// load vertices
	{
		this->occludedAttributes->addVertexAttribute(ChunkContainer::Vertices, 0, 2, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec2), 0);
	}

	// load uvs
	{
		this->occludedAttributes->addVertexAttribute(ChunkContainer::UVs, 1, 2, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec2), 0);
	}

	// load offsets
	{
		this->occludedBuffers[0]->setDynamicDraw(true);
		this->occludedBuffers[0]->setData(nullptr, 0, sizeof(glm::vec3));
		this->occludedAttributes->addVertexAttribute(this->occludedBuffers[0], 2, 3, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec3), 1);
	}

	// load texture indices
	{
		this->occludedBuffers[1]->setDynamicDraw(true);
		this->occludedBuffers[1]->setData(nullptr, 0, sizeof(int));
		this->occludedAttributes->addVertexAttribute(this->occludedBuffers[1], 3, 1, render::VERTEX_ATTRIB_INT, 0, sizeof(int), 1);
	}

	// load color indices
	{
		this->occludedBuffers[2]->setDynamicDraw(true);
		this->occludedBuffers[2]->setData(nullptr, 0, sizeof(glm::vec4));
		this->occludedAttributes->addVertexAttribute(this->occludedBuffers[2], 4, 4, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec4), 1);
	}

	// load occluded
	{
		this->occludedBuffers[3]->setDynamicDraw(true);
		this->occludedBuffers[3]->setData((int*)&Layer::Occluded, sizeof(int), sizeof(int));
		this->occludedAttributes->addVertexAttribute(this->occludedBuffers[3], 5, 1, render::VERTEX_ATTRIB_INT, 0, sizeof(int), 1);
	}
}

void Layer::rebuildBuffers() {
	this->Offsets.allocate(this->tiles.array.head);
	this->TextureIndices.allocate(this->tiles.array.head);
	this->Colors.allocate(this->tiles.array.head);

	// go through sorted list and build the texture/offset buffers
	this->occludedCount = 0;
	for(uint64_t i = 0; i < this->tiles.array.head; i++) {
		OverlappingTile* tile = this->tiles.array[i];
		this->Offsets[i] = tile->screenSpacePosition;
		this->TextureIndices[i] = tile->getTexture();
		this->Colors[i] = tile->getColor();

		if(tile->isOccluded()) {
			this->occludedCount++;
		}
	}

	this->buffers[0]->setData(&this->Offsets[0], sizeof(glm::vec3) * this->tiles.array.head, sizeof(glm::vec3));
	this->buffers[1]->setData(&this->TextureIndices[0], sizeof(int) * this->tiles.array.head, sizeof(int));
	this->buffers[2]->setData(&this->Colors[0], sizeof(glm::vec4) * this->tiles.array.head, sizeof(glm::vec4));

	this->OccludedEnabled.allocate(this->occludedCount);

	// handle occluded tiles
	for(uint64_t i = 0, j = 0; i < this->tiles.array.head; i++) {
		OverlappingTile* tile = this->tiles.array[i];
		if(!tile->isOccluded()) {
			continue;
		}

		this->Offsets[j] = tile->screenSpacePosition;
		this->TextureIndices[j] = tile->getTexture();
		this->Colors[j] = tile->getColor();
		this->OccludedEnabled[j] = 2;
		j++;
	}

	this->occludedBuffers[0]->setData(&this->Offsets[0], sizeof(glm::vec3) * this->occludedCount, sizeof(glm::vec3));
	this->occludedBuffers[1]->setData(&this->TextureIndices[0], sizeof(int) * this->occludedCount, sizeof(int));
	this->occludedBuffers[2]->setData(&this->Colors[0], sizeof(glm::vec4) * this->occludedCount, sizeof(glm::vec4));
	this->occludedBuffers[3]->setData(&this->OccludedEnabled[0], sizeof(int) * this->occludedCount, sizeof(int));
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

void Layer::renderOccluded(double deltaTime, RenderContext &context) {	
	this->occludedAttributes->bind();
	engine->renderWindow.enableDepthTest(false);
	engine->renderWindow.draw(render::PRIMITIVE_TRIANGLE_STRIP, 0, 4, 0, this->occludedCount);
	engine->renderWindow.enableDepthTest(true);
}
