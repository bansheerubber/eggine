#include "chunk.h"

#include <random>

#include "../basic/camera.h"
#include "chunkContainer.h"
#include "../util/doubleDimension.h"
#include "../engine/engine.h"
#include "../basic/line.h"
#include "overlappingTile.h"
#include "../resources/resourceManager.h"
#include "tileMath.h"

glm::vec2 Chunk::Offsets[Chunk::Size * Chunk::Size * 15];
render::VertexBuffer* Chunk::VertexBuffers[3] = {nullptr, nullptr, nullptr};

void initOverlappingTileWrapper(Chunk* chunk, OverlappingTileWrapper* tile) {
	*tile = {};
}

int compareOverlappingTile(const void* a, const void* b) {
	OverlappingTileWrapper* a2 = (OverlappingTileWrapper*)a;
	OverlappingTileWrapper* b2 = (OverlappingTileWrapper*)b;

	if(a2->index > b2->index) {
		return 1;
	}
	else if(a2->index < b2->index) {
		return -1;
	}
	else {
		return 0;
	}
}

Chunk::Chunk() : InstancedRenderObjectContainer(false) {
	if(Chunk::VertexBuffers[0] == nullptr) {
		// pre-calculate offsets
		{
			for(unsigned i = 0; i < Size * Size; i++) {
				for(unsigned z = 0; z < 15; z++) {
					glm::ivec2 coordinate = tilemath::indexToCoordinate(i, Size);
					Chunk::Offsets[i + z * Size * Size] = tilemath::tileToScreen(glm::vec3(coordinate, z));
				}
			}

			Chunk::VertexBuffers[0] = new render::VertexBuffer(&engine->renderWindow);
			Chunk::VertexBuffers[0]->setData(&Chunk::Offsets[0], sizeof(glm::vec2) * Chunk::Size * Chunk::Size * 15, alignof(glm::vec2));
		}

		// vertices for square
		{
			Chunk::VertexBuffers[1] = new render::VertexBuffer(&engine->renderWindow);
			Chunk::VertexBuffers[1]->setData((glm::vec2*)&Chunk::Vertices[0], sizeof(Chunk::Vertices), alignof(glm::vec2));
		}

		// uvs for square
		{
			Chunk::VertexBuffers[2] = new render::VertexBuffer(&engine->renderWindow);
			Chunk::VertexBuffers[2]->setData((glm::vec2*)&Chunk::UVs[0], sizeof(Chunk::UVs), alignof(glm::vec2));
		}
	}

	this->height = ((double)rand() / (RAND_MAX)) * 10 + 1;

	this->vertexBuffer = new render::VertexBuffer(&engine->renderWindow);
	this->vertexAttributes = new render::VertexAttributes(&engine->renderWindow);

	this->textureIndices = new int[Size * Size * this->height];
	for(unsigned i = 0; i < Size * Size * this->height; i++) {
		this->textureIndices[i] = 2;
	}
	
	// load vertices
	{
		this->vertexAttributes->addVertexAttribute(Chunk::VertexBuffers[1], 0, 2, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec2), 0);
	}

	// // load uvs
	{
		this->vertexAttributes->addVertexAttribute(Chunk::VertexBuffers[2], 1, 2, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec2), 0);
	}

	// // load offsets
	{
		this->vertexAttributes->addVertexAttribute(Chunk::VertexBuffers[0], 2, 2, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec2), 1);
	}

	// load texture indices
	{
		this->vertexBuffer->setData(this->textureIndices, sizeof(int) * Chunk::Size * Chunk::Size * this->height, alignof(int));
		this->vertexAttributes->addVertexAttribute(this->vertexBuffer, 3, 1, render::VERTEX_ATTRIB_INT, 0, sizeof(int), 1);
	}

	this->defineBounds();
}

Chunk::~Chunk() {
	size_t head = this->overlappingTiles.array.head;
	this->overlappingTiles.array.head = 0; // reset to zero to prevent double deallocations
	for(size_t i = 0; i < head; i++) {
		delete this->overlappingTiles.array[i].tile;
	}

	delete this->textureIndices;
	if(this->debugLine != nullptr) {
		delete this->debugLine;
	}
}

void Chunk::setPosition(glm::uvec2 position) {
	this->position = position;
	this->screenSpacePosition = tilemath::tileToScreen(glm::vec3((unsigned int)Size * this->position, 0.0));
	this->defineBounds();
}

glm::uvec2& Chunk::getPosition() {
	return this->position;
}

void Chunk::defineBounds() {
	glm::vec2 bias(-0.5, -(32.0 / 2.0 + 39.0 * 2.0 + 2) / 128.0);

	glm::vec2 top = tilemath::tileToScreen(glm::vec3(Size, 0, this->height)) + this->screenSpacePosition + bias;
	glm::vec2 right = tilemath::tileToScreen(glm::vec3(Size, Size, 0)) + this->screenSpacePosition + bias;
	glm::vec2 bottom = tilemath::tileToScreen(glm::vec3(0, Size, 0)) + this->screenSpacePosition + bias;
	glm::vec2 left = tilemath::tileToScreen(glm::vec3(0, 0, 0)) + this->screenSpacePosition + bias;

	this->left = left.x;
	this->right = right.x;
	this->top = top.y;
	this->bottom = bottom.y;
}

void Chunk::buildDebugLines() {
	this->debugLine = new Line();
	this->debugLine->addPosition(glm::vec2(this->left, this->bottom));
	this->debugLine->addPosition(glm::vec2(this->right, this->bottom));
	this->debugLine->addPosition(glm::vec2(this->right, this->top));
	this->debugLine->addPosition(glm::vec2(this->left, this->top));
	this->debugLine->addPosition(glm::vec2(this->left, this->bottom));
	this->debugLine->commit();
}

void Chunk::renderChunk(double deltaTime, RenderContext &context) {
	#ifdef EGGINE_DEBUG
	this->drawCalls = 0;
	#endif

	struct VertexBlock {
		glm::mat4 projection;
		glm::vec2 chunkScreenSpace;
		float spritesheetWidth;
		float spritesheetHeight;
		float spriteWidth;
		float spriteHeight;
		int spritesOnRow;
	} vb;
	vb.projection = context.camera->projectionMatrix;
	vb.chunkScreenSpace = this->screenSpacePosition;
	vb.spritesheetWidth = 1057;
	vb.spritesheetHeight = 391;
	vb.spriteWidth = 64.0f;
	vb.spriteHeight = 128.0f;
	vb.spritesOnRow = (int)vb.spritesheetWidth / (int)vb.spriteWidth;
	
	Camera* camera = context.camera;
	if(!(
		camera->left > this->right
		|| camera->right < this->left
		|| camera->top < this->bottom
		|| camera->bottom > this->top
	)) {
		unsigned int total = Chunk::Size * Chunk::Size * this->height;
		unsigned int lastOverlappingIndex = 0;
		size_t leftOff = 0;
		OverlappingTileWrapper* tile = nullptr;

		// handle overlapping tiles
		for(size_t i = 0; i < this->overlappingTiles.array.head && (tile = &this->overlappingTiles.array[i])->index < total; i++) { // go through overlapping tiles			
			int overlapBias = ChunkContainer::Image->drawOntopOfOverlap(this->textureIndices[tile->index]) ? 0 : 1;
			if(lastOverlappingIndex - 1 != tile->index) {
				// draw [last, lastOverlappingIndex - tile.index + last)
				// we need to reset the pipeline since we could have drawn an overlapping tile before this batch
				this->vertexAttributes->bind();
				ChunkContainer::Program->bindUniform("vertexBlock", &vb, sizeof(vb));
				engine->renderWindow.draw(render::PRIMITIVE_TRIANGLE_STRIP, 0, 4, lastOverlappingIndex, tile->index - lastOverlappingIndex + overlapBias);
				#ifdef EGGINE_DEBUG
				this->drawCalls++;
				#endif
			}

			tile->tile->render(deltaTime, context);

			lastOverlappingIndex = tile->index + overlapBias;

			leftOff = i + 1;
		}

		this->vertexAttributes->bind();
		ChunkContainer::Program->bindUniform("vertexBlock", &vb, sizeof(vb));
		engine->renderWindow.draw(render::PRIMITIVE_TRIANGLE_STRIP, 0, 4, lastOverlappingIndex, total - lastOverlappingIndex);
		#ifdef EGGINE_DEBUG
		this->drawCalls++;
		#endif

		// draw overlapping tiles above the height of the chunk
		for(size_t i = leftOff; i < this->overlappingTiles.array.head; i++) {
			this->overlappingTiles.array[i].tile->render(deltaTime, context);
		}

		#ifdef EGGINE_DEBUG
		this->isCulled = false;
		#endif
	}
	#ifdef EGGINE_DEBUG
	else {
		this->isCulled = true;
	}
	#endif
}

void Chunk::addOverlappingTile(OverlappingTile* tile) {
	glm::uvec2 relativePosition = glm::uvec2(tile->getPosition()) - this->position * (unsigned int)Chunk::Size;
	unsigned int index = tilemath::coordinateToIndex(relativePosition, Chunk::Size) + Chunk::Size * Chunk::Size * tile->getPosition().z;

	this->overlappingTiles.insert(OverlappingTileWrapper {
		index: index,
		tile: tile,
	});
}

void Chunk::updateOverlappingTile(OverlappingTile* tile) {
	// find the tile and update its index
	glm::uvec2 relativePosition = glm::uvec2(tile->getPosition()) - this->position * (unsigned int)Chunk::Size;
	unsigned int index = tilemath::coordinateToIndex(relativePosition, Chunk::Size) + Chunk::Size * Chunk::Size * tile->getPosition().z;

	for(size_t i = 0; i < this->overlappingTiles.array.head; i++) {
		if(this->overlappingTiles.array[i].tile == tile) {
			this->overlappingTiles.array[i].index = index;
		}
	}

	this->overlappingTiles.sort();
}
	
void Chunk::removeOverlappingTile(OverlappingTile* tile) {
	if(this->overlappingTiles.array.head != 0) { // signifies that the array has been deallocated
		this->overlappingTiles.remove(OverlappingTileWrapper {
			index: 0,
			tile: tile,
		});
	}
}
