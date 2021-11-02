#include "main.h"

#include <random>
#include <stdio.h>
#include <eggscript/egg.h>

#include "test/chunk.h"
#include "test/chunkContainer.h"
#include "engine/engine.h"
#include "basic/gameObject.h"
#include "basic/line.h"
#include "test/overlappingTile.h"
#include "carton/metadata/queryList.h"
#include "resources/resourceManager.h"
#include "test/team.h"
#include "basic/text.h"
#include "test/tileMath.h"
#include "test/unit.h"

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
	engine->chunkContainer = &container;

	size_t size = 5;
	for(size_t i = 0; i < size * size; i++) {
		container.addChunk(tilemath::indexToCoordinate(i, size));
	}

	container.commit();

	unsigned int positionsSize = 4;
	glm::uvec3 positions[positionsSize] = {
		glm::uvec3(0, 0, 5),
		glm::uvec3(2, 0, 5),
		glm::uvec3(3, 4, 5),
		glm::uvec3(7, 8, 5),
	};

	for(unsigned int i = 0; i < positionsSize; i++) {
		Unit* unit = new Unit(&container);
		unit->setPosition(positions[i]);
		container.getPlayerTeam()->add(unit);
	}

	engine->tick();

	return 0;
}
