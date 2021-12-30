#include "main.h"

#include <eggscript/egg.h>
#include <litehtml.h>
#include <random>
#include <stdio.h>

#include "test/chunk.h"
#include "test/chunkContainer.h"
#include "engine/engine.h"
#include "basic/gameObject.h"
#include "renderer/litehtmlContainer.h"
#include "basic/line.h"
#include "test/overlappingTile.h"
#include "resources/mapSource.h"
#include "carton/metadata/queryList.h"
#include "resources/resourceManager.h"
#include "test/team.h"
#include "basic/text.h"
#include "test/tileMath.h"
#include "test/unit.h"

#include "network/stream.h"

#include "sound/sound.h"

#ifdef __switch__
#include <switch.h>
#endif

int main(int argc, char* argv[]) {
	setbuf(stdout, NULL);
	
	#ifdef __switch__
	engine->setFilePrefix("romfs:/");
	#endif
	
	engine->initialize();

	// TODO do smarter loading of files
	engine->manager->loadResources(engine->manager->carton->database.get()->equals("extension", ".ss")->exec());

	ChunkContainer container;
	engine->chunkContainer = &container;

	resources::MapSource* map = (resources::MapSource*)engine->manager->loadResources(engine->manager->carton->database.get()->equals("extension", ".map")->exec())[0];
	map->loadIntoMap(&container.map);
	container.commit();

	unsigned int positionsSize = 4;
	glm::uvec3 positions[positionsSize] = {
		glm::uvec3(7, 4, 1),
		glm::uvec3(4, 8, 1),
		glm::uvec3(8, 10, 1),
		glm::uvec3(3, 15, 1),
	};

	for(unsigned int i = 0; i < positionsSize; i++) {
		Unit* unit = new Unit();
		unit->setAppearance(5);
		unit->setPosition(positions[i]);
		container.getPlayerTeam()->add(unit);
	}

	// enemies
	unsigned int enemyPositionsSize = 0;
	glm::uvec3 enemyPositions[enemyPositionsSize] = {
		glm::uvec3(2, 10, 1),
		glm::uvec3(4, 6, 1),
		glm::uvec3(3, 8, 1),
		glm::uvec3(1, 9, 1),
	};

	for(unsigned int i = 0; i < enemyPositionsSize; i++) {
		Unit* unit = new Unit();
		unit->setPosition(enemyPositions[i]);
		container.getEnemyTeam()->add(unit);
	}

	engine->tick();

	return 0;
}
