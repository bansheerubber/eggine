#include "main.h"

#include <glad/gl.h>
#include <glfw/glfw3.h>
#include <random>
#include <stdio.h>
#include <torquescript/ts.h>

#include "test/chunk.h"
#include "test/chunkContainer.h"
#include "engine/engine.h"
#include "basic/gameObject.h"
#include "basic/line.h"
#include "test/overlappingTile.h"
#include "carton/metadata/queryList.h"
#include "resources/resourceManager.h"
#include "basic/text.h"

#include "test/tileMath.h"

int main(int argc, char* argv[]) {
	engine->initialize();

	// TODO do smarter loading of files
	engine->manager.loadResources(engine->manager.carton->database.get()->has("fileName")->exec());

	ChunkContainer container;

	size_t size = 5;
	for(size_t i = 0; i < size * size; i++) {
		container.addChunk(tilemath::indexToCoordinate(i, size));
	}

	for(size_t i = 0; i < container.getChunkCount(); i++) {
		for(int egg = 0; egg < 0; egg++) {
			Chunk &chunk = container.getChunk(i);
			OverlappingTile* alien = new OverlappingTile();
			alien->setPosition(glm::uvec3(egg + chunk.getPosition().x * Chunk::Size, egg + chunk.getPosition().y * Chunk::Size, chunk.height - 1));
			container.addOverlappingTile(alien);
		}
	}

	engine->tick();

	return 0;
}
