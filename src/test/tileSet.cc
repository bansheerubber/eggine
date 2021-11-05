#include "tileSet.h"

#include <queue>

#include "../engine/engine.h"
#include "../util/manhattan.h"
#include "overlappingTile.h"

TileSet::TileSet() {
	this->reference = esInstantiateObject(engine->eggscript, "TileSet", this);
}

TileSet::~TileSet() {
	// esDeleteObject(this->reference);
	this->hideBorder();
}

bool TileSet::add(glm::uvec3 position) {
	bool alreadyHas = this->has(position);
	if(!alreadyHas) {
		this->set.insert(position);
	}
	return alreadyHas;
}

bool TileSet::remove(glm::uvec3 position) {
	bool alreadyHas = this->has(position);
	if(alreadyHas) {
		this->set.erase(position);
	}
	return alreadyHas;
}

bool TileSet::has(glm::uvec3 position) {
	return this->set.find(position) != this->set.end();
}

void TileSet::clear() {
	this->set.clear();
}

void TileSet::showDots() {
	this->hideBorder();
	for(glm::uvec3 position: this->set) {
		OverlappingTile* tile = (new OverlappingTile(engine->chunkContainer))
			->setTexture(30)
			->setPosition(position)
			->setZIndex(1);
		this->border.insert(tile);
	}
}

void TileSet::showBorder() {
	unsigned int tiles[4] = {19, 16, 12, 34};
	AdjacencyBit masks[4] = {ADJACENT_NORTH, ADJACENT_EAST, ADJACENT_SOUTH, ADJACENT_WEST};
	
	this->hideBorder();
	for(glm::uvec3 position: this->set) {
		AdjacencyBitmask bitmask = this->adjacency(position);
		for(unsigned int i = 0; i < 4; i++) {
			if(~bitmask & masks[i]) {
				OverlappingTile* tile = (new OverlappingTile(engine->chunkContainer))
					->setTexture(tiles[i])
					->setPosition(position)
					->setZIndex(1);
				this->border.insert(tile);
			}
		}
	}
}

void TileSet::hideBorder() {
	for(OverlappingTile* tile: this->border) {
		delete tile;
	}
	this->border.clear();
}

struct AStarEntry {
	glm::ivec3 position;
	unsigned int fScore = 0;
	
	friend bool operator<(const AStarEntry &left, const AStarEntry &right) {
		return left.fScore > right.fScore;
	}
};

TileSet* TileSet::pathfind(glm::ivec3 start, glm::ivec3 end) {
	if(!this->has(start) || !this->has(end)) {
		return nullptr;
	}

	glm::ivec3 offsets[4] = {
		glm::ivec3(1, 0, 0),
		glm::ivec3(-1, 0, 0),
		glm::ivec3(0, 1, 0),
		glm::ivec3(0, -1, 0),
	};

	tsl::robin_map<glm::ivec3, unsigned int> distances;
	tsl::robin_map<glm::ivec3, glm::ivec3> cameFrom;
	priority_queue<AStarEntry> queue;
	auto heuristic = [](glm::ivec3 a, glm::ivec3 b) { return (unsigned int)manhattan(a, b); };

	queue.push(AStarEntry {
		position: start,
		fScore: heuristic(start, end),
	});

	TileSet* output = new TileSet();
	while(queue.size() != 0) {
		AStarEntry entry = queue.top();
		queue.pop();

		if(entry.position == end) {
			output->add(end);
			glm::ivec3 current = end;
			while(cameFrom.find(current) != cameFrom.end()) {
				current = cameFrom[current];
				output->add(current);
			}
			return output;
		}

		// TODO generalize neighbor test for unit movement and pathfinding
		for(unsigned int i = 0; i < 4; i++) {
			glm::ivec3 neighbor = entry.position + offsets[i];
			if(!this->has(neighbor)) {
				continue;
			}

			unsigned int test = distances[entry.position] + 1;
			auto found = distances.find(neighbor);
			if(found == distances.end() || test < found.value()) {
				cameFrom[neighbor] = entry.position;
				distances[neighbor] = test;
				queue.push(AStarEntry {
					position: neighbor,
					fScore: distances[neighbor] + heuristic(neighbor, end),
				});
			}
		}
	}

	return output;
}

AdjacencyBitmask TileSet::adjacency(glm::ivec3 position) {
	glm::ivec3 offsets[4] = {
		glm::ivec3(0, -1, 0), // north
		glm::ivec3(1, 0, 0), // east
		glm::ivec3(0, 1, 0), // south
		glm::ivec3(-1, 0, 0), // west
	};

	AdjacencyBit masks[4] = {ADJACENT_NORTH, ADJACENT_EAST, ADJACENT_SOUTH, ADJACENT_WEST};

	AdjacencyBitmask result = 0;
	for(unsigned int i = 0; i < 4; i++) {
		if(this->has(position + offsets[i])) {
			result |= masks[i];
		}
	}
	return result;
}

void es::defineTileSet() {
	esRegisterNamespace(engine->eggscript, "TileSet");
	esNamespaceInherit(engine->eggscript, "SimObject", "TileSet");

	esEntryType addArguments[2] = {ES_ENTRY_OBJECT, ES_ENTRY_MATRIX};
	esRegisterMethod(engine->eggscript, ES_ENTRY_NUMBER, es::TileSet__add, "TileSet", "add", 2, addArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_NUMBER, es::TileSet__remove, "TileSet", "remove", 2, addArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_NUMBER, es::TileSet__has, "TileSet", "has", 2, addArguments);

	esEntryType clearArguments[1] = {ES_ENTRY_OBJECT};
	esRegisterMethod(engine->eggscript, ES_ENTRY_INVALID, es::TileSet__clear, "TileSet", "clear", 1, clearArguments);

	esRegisterMethod(engine->eggscript, ES_ENTRY_INVALID, es::TileSet__showDots, "TileSet", "showDots", 1, clearArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_INVALID, es::TileSet__showBorder, "TileSet", "showBorder", 1, clearArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_INVALID, es::TileSet__hideBorder, "TileSet", "hideBorder", 1, clearArguments);
}

esEntryPtr es::TileSet__add(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 2 && esCompareNamespaceToObject(args[0].objectData, "TileSet") && args[1].matrixData->rows == 3 && args[1].matrixData->columns == 1) {
		esEntryPtr entry = new esEntry();
		entry->type = ES_ENTRY_NUMBER;
		entry->numberData = ((TileSet*)args[0].objectData->objectWrapper->data)->add(
			glm::uvec3(args[1].matrixData->data[0][0].numberData, args[1].matrixData->data[1][0].numberData, args[1].matrixData->data[2][0].numberData)
		);
		return entry;
	}

	return nullptr;
}

esEntryPtr es::TileSet__remove(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 2 && esCompareNamespaceToObject(args[0].objectData, "TileSet") && args[1].matrixData->rows == 3 && args[1].matrixData->columns == 1) {
		esEntryPtr entry = new esEntry();
		entry->type = ES_ENTRY_NUMBER;
		entry->numberData = ((TileSet*)args[0].objectData->objectWrapper->data)->remove(
			glm::uvec3(args[1].matrixData->data[0][0].numberData, args[1].matrixData->data[1][0].numberData, args[1].matrixData->data[2][0].numberData)	
		);
		return entry;
	}

	return nullptr;
}

esEntryPtr es::TileSet__has(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 2 && esCompareNamespaceToObject(args[0].objectData, "TileSet") && args[1].matrixData->rows == 3 && args[1].matrixData->columns == 1) {
		return esCreateNumber(
			((TileSet*)args[0].objectData->objectWrapper->data)->has(
				glm::uvec3(args[1].matrixData->data[0][0].numberData, args[1].matrixData->data[1][0].numberData, args[1].matrixData->data[2][0].numberData)
			)	
		);
	}

	return nullptr;
}

esEntryPtr es::TileSet__clear(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 1 && esCompareNamespaceToObject(args[0].objectData, "TileSet")) {
		((TileSet*)args[0].objectData->objectWrapper->data)->clear();
	}

	return nullptr;
}

esEntryPtr es::TileSet__showDots(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 1 && esCompareNamespaceToObject(args[0].objectData, "TileSet")) {
		((TileSet*)args[0].objectData->objectWrapper->data)->showDots();
	}

	return nullptr;
}

esEntryPtr es::TileSet__showBorder(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 1 && esCompareNamespaceToObject(args[0].objectData, "TileSet")) {
		((TileSet*)args[0].objectData->objectWrapper->data)->showBorder();
	}

	return nullptr;
}

esEntryPtr es::TileSet__hideBorder(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 1 && esCompareNamespaceToObject(args[0].objectData, "TileSet")) {
		((TileSet*)args[0].objectData->objectWrapper->data)->hideBorder();
	}

	return nullptr;
}
