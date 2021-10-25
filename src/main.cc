#include "main.h"

#include <random>
#include <stdio.h>
#include <eggscript/egg.h>

#include "test/character.h"
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

#ifdef __switch__
#include <switch.h>
#endif

int main(int argc, char* argv[]) {
	#ifdef __switch__
	engine->setFilePrefix("romfs:/");
	#endif
	
	engine->initialize();

	// TODO do smarter loading of files
	engine->manager->loadResources(engine->manager->carton->database.get()->equals("extension", ".png")->exec());

	ChunkContainer container;

	size_t size = 25;
	for(size_t i = 0; i < size * size; i++) {
		container.addChunk(tilemath::indexToCoordinate(i, size));
	}

	Character* character = new Character(&container);
	character->setPosition(glm::uvec3(0, 0, 10)); 

	engine->tick();

	return 0;
}
