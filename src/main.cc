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
#include "basic/pngImage.h"
#include "basic/text.h"

#include "test/tileMath.h"

int main(int argc, char* argv[]) {
	engine->initialize();

	ChunkContainer container;

	size_t size = 100;
	for(size_t i = 0; i < size * size; i++) {
		container.addChunk(new Chunk(tilemath::indexToCoordinate(i, size)));
	}

	container.buildRenderOrder();

	for(Chunk* chunk: container.chunks) {
		OverlappingTile* alien = new OverlappingTile();
		alien->setPosition(glm::uvec3(5 + chunk->position.x * Chunk::Size, 5 + chunk->position.y * Chunk::Size, chunk->height - 1));
		container.addOverlappingTile(alien);	
	}

	engine->tick();

	return 0;
}
