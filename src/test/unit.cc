#include "unit.h"

#include <queue>

#include "chunkContainer.h"
#include "../util/manhattan.h"
#include "../util/minHeap.h"
#include "team.h"

Unit::Unit(bool createReference) : Character(false) {
	if(createReference) {
		this->reference = esInstantiateObject(engine->eggscript, "Unit", this);
	}
}

Unit::~Unit() {
	esDeleteObject(this->reference);

	if(engine->chunkContainer->getSelectedCharacter() == this) {
		engine->chunkContainer->selectCharacter(nullptr);
	}

	if(this->team != nullptr) {
		this->team->remove(this);
	}

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

void Unit::calculateDestinations(TileSet &destinations, unsigned int moves) {
	destinations.clear();

	glm::ivec3 start = this->getPosition();
	if(start.z > 0) {
		start.z -= 1;
	}

	std::priority_queue<DijkstraEntry> queue;
	tsl::robin_map<glm::vec3, unsigned int> distances;

	destinations.add(start);

	queue.push(DijkstraEntry {
		position: start,
		moves: moves,
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
			if(test <= moves && (found == distances.end() || test < found.value())) {
				distances[neighbor] = test;
				queue.push(DijkstraEntry {
					position: neighbor,
					moves: test,
				});
				destinations.add(neighbor);
			}
		}
	}
}

TileSet* Unit::getPath(glm::ivec3 end) {
	glm::ivec3 start = this->getPosition();
	if(start.z > 0) {
		start.z -= 1;
	}

	if(start == this->pathCache.start && end == this->pathCache.end) {
		return this->path;
	}

	if(this->path != nullptr) {
		delete this->path;
	}

	this->pathCache.start = start;
	this->pathCache.end = end;

	this->path = this->destinations.pathfind(start, end);
	if(this->path != nullptr) {
		return this->path;
	}
	else {
		return this->path = this->sprintDestinations.pathfind(start, end);
	}
}

void Unit::kill() {
	delete this;
}

OverlappingTile* Unit::setPosition(glm::uvec3 position) {
	Character::setPosition(position);
	this->healthbar.setPosition(position);
	return this;
}

void Unit::setHealth(int health) {
	if(this->unpacking && health <= 0) {
		throw network::RemoteObjectUnpackException(this, "Unit: invalid max health");
	}
	
	esEntry arguments[2];
	esCreateObjectAt(&arguments[0], this->reference);
	esCreateNumberAt(&arguments[1], health);
	esDeleteEntry(esCallMethod(engine->eggscript, this->reference, "setHealth", 2, arguments));
}

void Unit::setMaxHealth(int maxHealth) {
	if(this->unpacking && maxHealth <= 0) {
		throw network::RemoteObjectUnpackException(this, "Unit: invalid max health");
	}
	
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

	esEntryType destinationArguments[2] = {ES_ENTRY_OBJECT};
	esRegisterMethod(engine->eggscript, ES_ENTRY_OBJECT, es::Unit__getDestinations, "Unit", "getDestinations", 1, destinationArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_OBJECT, es::Unit__getSprintDestinations, "Unit", "getSprintDestinations", 1, destinationArguments);

	esEntryType getPathArguments[1] = {ES_ENTRY_OBJECT};
	esRegisterMethod(engine->eggscript, ES_ENTRY_OBJECT, es::Unit__getPath, "Unit", "getPath", 1, getPathArguments);

	esEntryType setHealthArguments[2] = {ES_ENTRY_OBJECT, ES_ENTRY_NUMBER};
	esRegisterMethod(engine->eggscript, ES_ENTRY_EMPTY, es::Unit__setHealth, "Unit", "setHealth", 2, setHealthArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_EMPTY, es::Unit__setMaxHealth, "Unit", "setMaxHealth", 2, setHealthArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_EMPTY, es::Unit__addHealth, "Unit", "addHealth", 2, setHealthArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_EMPTY, es::Unit__addMaxHealth, "Unit", "addMaxHealth", 2, setHealthArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_NUMBER, es::Unit__getHealth, "Unit", "getHealth", 1, getPathArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_NUMBER, es::Unit__getMaxHealth, "Unit", "getMaxHealth", 1, getPathArguments);

	esRegisterMethod(engine->eggscript, ES_ENTRY_OBJECT, es::Unit__getTeam, "Unit", "getTeam", 1, getPathArguments);
}

esEntryPtr es::Unit__getDestinations(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 1 && esCompareNamespaceToObject(args[0].objectData, "Unit")) {
		Unit* unit = (Unit*)args[0].objectData->objectWrapper->data;
		unit->calculateDestinations(unit->destinations, esGetNumberFromEntry(esGetObjectProperty(unit->reference, "moves")));
		return esCreateObject(unit->destinations.reference);
	}
	return nullptr;
}

esEntryPtr es::Unit__getSprintDestinations(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 1 && esCompareNamespaceToObject(args[0].objectData, "Unit")) {
		Unit* unit = (Unit*)args[0].objectData->objectWrapper->data;
		unit->calculateDestinations(unit->sprintDestinations, esGetNumberFromEntry(esGetObjectProperty(unit->reference, "sprintMoves")));
		return esCreateObject(unit->sprintDestinations.reference);
	}
	return nullptr;
}

esEntryPtr es::Unit__getPath(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 2 && esCompareNamespaceToObject(args[0].objectData, "Unit") && args[1].matrixData != nullptr && args[1].matrixData->rows == 3 && args[1].matrixData->columns == 1) {
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
	if(argc == 1 && esCompareNamespaceToObject(args[0].objectData, "Unit")) {
		((Unit*)args[0].objectData->objectWrapper->data)->kill();
	}
	return nullptr;
}

esEntryPtr es::Unit__setHealth(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 2 && esCompareNamespaceToObject(args[0].objectData, "Unit")) {
		Unit* unit = (Unit*)args[0].objectData->objectWrapper->data;
		unit->health = (int)args[1].numberData;
		unit->healthbar.setPercent((double)unit->health / (double)unit->maxHealth);

		unit->writeUpdateMask("health");

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
		unit->health = std::max(unit->maxHealth, unit->health);
		unit->healthbar.setPercent((double)unit->health / (double)unit->maxHealth);

		unit->writeUpdateMask("maxHealth");
	}
	return nullptr;
}

esEntryPtr es::Unit__addHealth(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 2 && esCompareNamespaceToObject(args[0].objectData, "Unit")) {
		Unit* unit = (Unit*)args[0].objectData->objectWrapper->data;
		unit->health = std::max(std::min(unit->health + (int)args[1].numberData, unit->maxHealth), 0);
		unit->healthbar.setPercent((double)unit->health / (double)unit->maxHealth);
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
		unit->healthbar.setPercent((double)unit->health / (double)unit->maxHealth);
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

esEntryPtr es::Unit__getTeam(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 1 && esCompareNamespaceToObject(args[0].objectData, "Unit")) {
		Unit* unit = (Unit*)args[0].objectData->objectWrapper->data;
		return esCreateObject(unit->team->reference);
	}
	return nullptr;
}
