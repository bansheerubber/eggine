#include "main.h"

#include <glad/gl.h>
#include <glfw/glfw3.h>
#include <random>
#include <stdio.h>

#include "test/chunk.h"
#include "test/chunkContainer.h"
#include "engine/engine.h"
#include "basic/gameObject.h"
#include "basic/line.h"
#include "basic/pngImage.h"
#include "basic/text.h"

#include "test/tileMath.h"

int main(int argc, char* argv[]) {
	engine->initialize();

	ChunkContainer container;

	size_t size = 5;
	size_t total = 0;
	for(size_t i = 0; i < size * size; i++) {
		Chunk* chunk = new Chunk(tilemath::indexToCoordinate(i, size));
		total += chunk->height + Chunk::Size * Chunk::Size;
		container.addChunk(chunk);
	}
	printf("%ld\n", total);

	container.buildRenderOrder();

	engine->tick();

	return 0;
}
