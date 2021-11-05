#include "unit.h"

#include <queue>

#include "chunkContainer.h"
#include "../util/manhattan.h"
#include "../util/minHeap.h"

Unit::Unit(ChunkContainer* chunkContainer, bool createReference) : Character(chunkContainer, false) {
	if(createReference) {
		this->reference = esInstantiateObject(engine->eggscript, "Unit", this);
	}
}

Unit::~Unit() {
	esDeleteObject(this->reference);

	if(this->path != nullptr) {
		delete this->path;
	}
}

void Unit::move(glm::ivec3 position) {
	if(this->container->isValidTilePosition(position) && this->destinations.has(position)) {
		this->setPosition(position);
	}
}

struct DijkstraEntry {
	glm::ivec3 position;
	unsigned int moves = 0;
	
	friend bool operator<(const DijkstraEntry &left, const DijkstraEntry &right) {
		return left.moves > right.moves;
	}
};

void Unit::calculateDestinations() {
	this->destinations.clear();

	this->lastDestinationsCalculation.moves = this->moves;
	this->lastDestinationsCalculation.position = this->getPosition(); // different than start

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

	while(queue.size() != 0) {
		DijkstraEntry entry = queue.top();
		queue.pop();
		for(unsigned int i = 0; i < 4; i++) {
			glm::ivec3 neighbor = entry.position + offsets[i];
			if(!this->container->isValidTilePosition(neighbor)) {
				continue;
			}

			unsigned int test = distances[entry.position] + 1;
			auto found = distances.find(neighbor);
			if(test <= this->moves && (found == distances.end() || test < found.value())) {
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

TileSet* Unit::getPath(glm::ivec3 end) {
	if(this->path != nullptr) {
		delete this->path;
	}
	
	this->calculateDestinations();
	glm::ivec3 start = this->getPosition();
	if(start.z > 0) {
		start.z -= 1;
	}
	return this->path = this->destinations.pathfind(start, end);
}

void es::defineUnit() {
	esRegisterNamespace(engine->eggscript, "Unit");
	esNamespaceInherit(engine->eggscript, "Character", "Unit");

	esEntryType destinationArguments[1] = {ES_ENTRY_OBJECT};
	esRegisterMethod(engine->eggscript, ES_ENTRY_OBJECT, es::Unit__getDestinations, "Unit", "getDestinations", 1, destinationArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_OBJECT, es::Unit__getPath, "Unit", "getPath", 1, destinationArguments);

	esEntryType setMovesArguments[2] = {ES_ENTRY_OBJECT, ES_ENTRY_NUMBER};
	esRegisterMethod(engine->eggscript, ES_ENTRY_INVALID, es::Unit__setMoves, "Unit", "setMoves", 2, setMovesArguments);
}

esEntryPtr es::Unit__setMoves(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 2 && esCompareNamespaceToObject(args[0].objectData, "Unit")) {
		Unit* unit = (Unit*)args[0].objectData->objectWrapper->data;
		unit->moves = args[1].numberData;
	}
	return nullptr;
}

esEntryPtr es::Unit__getDestinations(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 1 && esCompareNamespaceToObject(args[0].objectData, "Unit")) {
		Unit* unit = (Unit*)args[0].objectData->objectWrapper->data;
		if(unit->getPosition() != unit->lastDestinationsCalculation.position || unit->moves != unit->lastDestinationsCalculation.moves) {
			unit->calculateDestinations();
		}

		return esCreateObject(unit->destinations.reference);
	}
	return nullptr;
}

esEntryPtr es::Unit__getPath(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 2 && esCompareNamespaceToObject(args[0].objectData, "Unit") && args[1].matrixData->rows == 3 && args[1].matrixData->columns == 1) {
		Unit* unit = (Unit*)args[0].objectData->objectWrapper->data;
		unit->getPath(glm::ivec3(
			args[1].matrixData->data[0][0].numberData,
			args[1].matrixData->data[1][0].numberData,
			args[1].matrixData->data[2][0].numberData
		));

		if(unit->path) {
			return esCreateObject(unit->path->reference);
		}
	}
	return nullptr;
}
