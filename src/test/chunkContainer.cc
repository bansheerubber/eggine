#include "chunkContainer.h"

#define FMT_HEADER_ONLY
#include <fmt/format.h>

#include "../basic/camera.h"
#include "chunk.h"
#include "../engine/debug.h"
#include "../engine/engine.h"
#include "overlappingTile.h"
#include "tileMath.h"

// Shader* ChunkContainer::Program = nullptr;
resources::SpriteSheet* ChunkContainer::Image = nullptr;

void initChunk(class ChunkContainer* container, class Chunk* chunk) {
	new((void*)chunk) Chunk();
}

ChunkContainer::ChunkContainer() {
	// if(ChunkContainer::Program == nullptr) {
	// 	ChunkContainer::Program = new Shader("shaders/tile.vert", "shaders/tile.frag");
	// }

	if(ChunkContainer::Image == nullptr) {
		ChunkContainer::Image = (resources::SpriteSheet*)engine->manager->metadataToResources(
			engine->manager->carton->database.get()->equals("extension", ".png")->exec()
		)[0];
	}
}

ChunkContainer::~ChunkContainer() {
	for(size_t i = 0; i < this->renderOrder.head; i++) {
		this->renderOrder[i].~Chunk(); // because of how dynamic array reallocs we have to do this bullshit
	}
}

void ChunkContainer::addChunk(glm::uvec2 position) {
	this->renderOrder[this->renderOrder.head].setPosition(position);
	this->renderOrder.pushed();

	this->size = ceil(sqrt(this->renderOrder.head));
}

Chunk& ChunkContainer::getChunk(size_t index) {
	if(index >= this->size * this->size) {
		printf("ChunkContainer::getChunk(): chunk index out of bounds\n");
		exit(1);
	}

	return this->renderOrder[index];
}

size_t ChunkContainer::getChunkCount() {
	return this->renderOrder.head;
}

void ChunkContainer::render(double deltaTime, RenderContext &context) {
	// ChunkContainer::Program->bind();

	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_2D, ChunkContainer::Image->texture);
	// glUniform1i(ChunkContainer::Program->getUniform("spriteTexture"), 0); // bind texture	
	// glUniformMatrix4fv(ChunkContainer::Program->getUniform("projection"), 1, false, &context.camera->projectionMatrix[0][0]);

	#ifdef EGGINE_DEBUG
	size_t chunksRendered = 0;
	size_t tilesRendered = 0;
	size_t tiles = 0;
	size_t drawCalls = 0;
	size_t overlappingCalls = 0;
	#endif

	for(size_t i = 0; i < this->renderOrder.head; i++) {
		Chunk &chunk = this->renderOrder[i];
		chunk.renderChunk(deltaTime, context);

		#ifdef EGGINE_DEBUG
		if(!chunk.isCulled) {
			chunksRendered++;
			tilesRendered += Chunk::Size * Chunk::Size * chunk.height;
			drawCalls += chunk.drawCalls;
			overlappingCalls += chunk.overlappingTiles.array.head;
		}
		
		tiles += Chunk::Size * Chunk::Size * chunk.height;
		#endif
	}

	#ifdef EGGINE_DEBUG
	engine->debug.addInfoMessage(fmt::format("{}/{} chunks rendered", chunksRendered, this->renderOrder.head));
	engine->debug.addInfoMessage(fmt::format("{}/{} tiles rendered", tilesRendered, tiles));
	engine->debug.addInfoMessage(fmt::format("{} chunk draw calls, {} overlapping calls", drawCalls, overlappingCalls));
	#endif
}

void ChunkContainer::addOverlappingTile(OverlappingTile* tile) {
	this->setOverlappingTileChunk(tile);
}

void ChunkContainer::setOverlappingTileChunk(OverlappingTile* tile) {
	glm::uvec2 chunkPosition = tile->getPosition() / (unsigned int)Chunk::Size;

	if(chunkPosition.x >= this->size || chunkPosition.y >= this->size) {
		printf("ChunkContainer::setOverlappingTileChunk(): chunk position out of bounds\n");
		exit(1);
	}

	long index = tilemath::coordinateToIndex(chunkPosition, this->size);
	this->renderOrder[index].addOverlappingTile(tile);
}
