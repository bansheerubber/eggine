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

	while(queue.size() != 0) {
		DijkstraEntry entry = queue.top();
		queue.pop();

		for(auto it = this->container->getNeighbors(entry.position); !it.atEnd(); ++it) {
			glm::ivec3 neighbor = it.value();
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

void Unit::kill() {

}

void Unit::setHealth(int health) {
	esEntry arguments[2];
	esCreateObjectAt(&arguments[0], this->reference);
	esCreateNumberAt(&arguments[1], health);
	esDeleteEntry(esCallMethod(engine->eggscript, this->reference, "setHealth", 2, arguments));
}

void Unit::setMaxHealth(int maxHealth) {
	esEntry arguments[2];
	esCreateObjectAt(&arguments[0], this->reference);
	esCreateNumberAt(&arguments[1], maxHealth);
	esDeleteEntry(esCallMethod(engine->eggscript, this->reference, "setMaxHealth", 2, arguments));
}

void Unit::addHealth(int health) {
	esEntry arguments[2];
	esCreateObjectAt(&arguments[0], this->reference);
	esCreateNumberAt(&arguments[1], health);
	esDeleteEntry(esCallMethod(engine->eggscript, this->reference, "addHealth", 2, arguments));
}

void Unit::addMaxHealth(int maxHealth) {
	esEntry arguments[2];
	esCreateObjectAt(&arguments[0], this->reference);
	esCreateNumberAt(&arguments[1], maxHealth);
	esDeleteEntry(esCallMethod(engine->eggscript, this->reference, "addMaxHealth", 2, arguments));
}

int Unit::getHealth() {
	esEntry arguments[1];
	esCreateObjectAt(&arguments[0], this->reference);
	esEntryPtr entry = esCallMethod(engine->eggscript, this->reference, "getHealth", 1, arguments);
	int health = entry->numberData;
	esDeleteEntry(entry);
	return health;
}

int Unit::getMaxHealth() {
	esEntry arguments[1];
	esCreateObjectAt(&arguments[0], this->reference);
	esEntryPtr entry = esCallMethod(engine->eggscript, this->reference, "getMaxHealth", 1, arguments);
	int health = entry->numberData;
	esDeleteEntry(entry);
	return health;
}

void es::defineUnit() {
	esRegisterNamespace(engine->eggscript, "Unit");
	esNamespaceInherit(engine->eggscript, "Character", "Unit");

	esEntryType destinationArguments[2] = {ES_ENTRY_OBJECT, ES_ENTRY_NUMBER};
	esRegisterMethod(engine->eggscript, ES_ENTRY_OBJECT, es::Unit__getDestinations, "Unit", "getDestinations", 2, destinationArguments);

	esEntryType getPathArguments[1] = {ES_ENTRY_OBJECT};
	esRegisterMethod(engine->eggscript, ES_ENTRY_OBJECT, es::Unit__getPath, "Unit", "getPath", 1, getPathArguments);

	esEntryType setMovesArguments[2] = {ES_ENTRY_OBJECT, ES_ENTRY_NUMBER};
	esRegisterMethod(engine->eggscript, ES_ENTRY_EMPTY, es::Unit__setMoves, "Unit", "setMoves", 2, setMovesArguments);

	esRegisterMethod(engine->eggscript, ES_ENTRY_EMPTY, es::Unit__setHealth, "Unit", "setHealth", 2, setMovesArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_EMPTY, es::Unit__setMaxHealth, "Unit", "setMaxHealth", 2, setMovesArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_EMPTY, es::Unit__addHealth, "Unit", "addHealth", 2, setMovesArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_EMPTY, es::Unit__addMaxHealth, "Unit", "addMaxHealth", 2, setMovesArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_NUMBER, es::Unit__getHealth, "Unit", "getHealth", 1, getPathArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_NUMBER, es::Unit__getMaxHealth, "Unit", "getMaxHealth", 1, getPathArguments);
}

esEntryPtr es::Unit__setMoves(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 2 && esCompareNamespaceToObject(args[0].objectData, "Unit")) {
		Unit* unit = (Unit*)args[0].objectData->objectWrapper->data;
		unit->moves = args[1].numberData;
	}
	return nullptr;
}

esEntryPtr es::Unit__getDestinations(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc >= 1 && esCompareNamespaceToObject(args[0].objectData, "Unit")) {
		Unit* unit = (Unit*)args[0].objectData->objectWrapper->data;
		if(args[1].numberData == 1 || unit->getPosition() != unit->lastDestinationsCalculation.position || unit->moves != unit->lastDestinationsCalculation.moves) {
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

esEntryPtr es::Unit__kill(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	return nullptr;
}

esEntryPtr es::Unit__setHealth(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 2 && esCompareNamespaceToObject(args[0].objectData, "Unit")) {
		Unit* unit = (Unit*)args[0].objectData->objectWrapper->data;
		unit->health = (int)args[1].numberData;

		if(unit->health <= 0) {
			unit->kill();
		}
	}
	return nullptr;
}

esEntryPtr es::Unit__setMaxHealth(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 2 && esCompareNamespaceToObject(args[0].objectData, "Unit")) {
		Unit* unit = (Unit*)args[0].objectData->objectWrapper->data;
		unit->maxHealth = (int)args[1].numberData;
		unit->health = max(unit->maxHealth, unit->health);
	}
	return nullptr;
}

esEntryPtr es::Unit__addHealth(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 2 && esCompareNamespaceToObject(args[0].objectData, "Unit")) {
		Unit* unit = (Unit*)args[0].objectData->objectWrapper->data;
		unit->health = max(min(unit->health + (int)args[1].numberData, unit->maxHealth), 0);
		if(unit->health <= 0) {
			unit->kill();
		}
	}
	return nullptr;
}

esEntryPtr es::Unit__addMaxHealth(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 2 && esCompareNamespaceToObject(args[0].objectData, "Unit")) {
		Unit* unit = (Unit*)args[0].objectData->objectWrapper->data;
		unit->maxHealth = unit->maxHealth + (int)args[1].numberData;
	}
	return nullptr;
}

esEntryPtr es::Unit__getHealth(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 1 && esCompareNamespaceToObject(args[0].objectData, "Unit")) {
		Unit* unit = (Unit*)args[0].objectData->objectWrapper->data;
		return esCreateNumber(unit->health);
	}
	return nullptr;
}

esEntryPtr es::Unit__getMaxHealth(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 1 && esCompareNamespaceToObject(args[0].objectData, "Unit")) {
		Unit* unit = (Unit*)args[0].objectData->objectWrapper->data;
		return esCreateNumber(unit->maxHealth);
	}
	return nullptr;
}
