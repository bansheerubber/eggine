#include "unit.h"

#include <queue>

#include "chunkContainer.h"
#include "../util/manhattan.h"
#include "../util/minHeap.h"

Unit::Unit(ChunkContainer* chunkContainer) : Character(chunkContainer) {
	this->reference = esInstantiateObject(engine->eggscript, "Unit", this);
}

Unit::~Unit() {
	esDeleteObject(this->reference);
}

struct DijkstraEntry {
	glm::ivec3 position;
	unsigned int moves = 0;
	
	friend bool operator<(const DijkstraEntry &left, const DijkstraEntry &right) {
		return left.moves < right.moves;
	}
};

void Unit::calculateDestinations(glm::ivec3 tilePosition, unsigned int move) {
	this->destinations.clear();

	glm::ivec3 start = this->getPosition();
	if(start.z > 0) {
		start.z -= 1;
	}

	priority_queue<DijkstraEntry> queue;
	tsl::robin_map<glm::vec3, unsigned int> distances;

	this->destinations.add(start);

	queue.push(DijkstraEntry {
		position: start,
		moves: 5,
	});

	glm::ivec3 offsets[4] = {
		glm::ivec3(1, 0, 0),
		glm::ivec3(-1, 0, 0),
		glm::ivec3(0, 1, 0),
		glm::ivec3(0, -1, 0),
	};

	unsigned int count = 0;
	while(queue.size() != 0) {
		DijkstraEntry entry = queue.top();
		queue.pop();
		count++;
		for(unsigned int i = 0; i < 4; i++) {
			glm::ivec3 neighbor = entry.position + offsets[i];
			if(!this->container->isValidTilePosition(neighbor)) {
				continue;
			}

			unsigned int test = distances[entry.position] + 1;
			auto found = distances.find(neighbor);
			if(test < this->moves && (found == distances.end() || test < found.value())) {
				distances[neighbor] = test;
				queue.push(DijkstraEntry {
					position: neighbor,
					moves: test,
				});
				this->destinations.add(neighbor);
			}
		}
	}
}

void es::defineUnit() {
	esRegisterNamespace(engine->eggscript, "Unit");
	esNamespaceInherit(engine->eggscript, "Character", "Unit");

	esEntryType destinationArguments[1] = {ES_ENTRY_OBJECT};
	esRegisterMethod(engine->eggscript, ES_ENTRY_OBJECT, es::Unit__getDestinations, "Unit", "getDestinations", 1, destinationArguments);
}

esEntryPtr es::Unit__getDestinations(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 1 && esCompareNamespaceToObject(args[0].objectData, "Unit")) {
		Unit* unit = (Unit*)args[0].objectData->objectWrapper->data;
		unit->calculateDestinations();
		return esCreateObject(unit->destinations.reference);
	}
	return nullptr;
}
