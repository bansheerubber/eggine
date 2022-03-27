#include "tileSet.h"

#include <queue>

#include "chunkContainer.h"
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
	return this->set.contains(position);
}

unsigned int TileSet::size() {
	return this->set.size();
}

void TileSet::clear() {
	this->set.clear();
	this->cameFrom.clear();
	this->wentTo.clear();
}

void TileSet::showDots() {
	this->hideBorder();
	for(glm::uvec3 position: this->set) {
		OverlappingTile* tile = (new OverlappingTile(engine->chunkContainer))
			->setTexture(30)
			->setPosition(position)
			->setZIndex(1)
			->setXRay(2);
		this->border.insert(tile);
	}
}

void TileSet::showBorder(glm::vec4 color) {
	unsigned int tiles[4] = {19, 16, 12, 34};
	AdjacencyBit masks[4] = {ADJACENT_NORTH, ADJACENT_EAST, ADJACENT_SOUTH, ADJACENT_WEST};
	
	this->hideBorder();
	for(glm::uvec3 position: this->set) {
		AdjacencyBitmask bitmask = this->adjacency(position);
		for(unsigned int i = 0; i < 4; i++) {
			if(~bitmask & masks[i]) {
				int textureIndex = i - engine->chunkContainer->getRotation();
				if(textureIndex < 0) {
					textureIndex += 4;
				}

				OverlappingTile* tile = (new OverlappingTile(engine->chunkContainer))
					->setTexture(tiles[textureIndex])
					->setPosition(position)
					->setZIndex(1)
					->setColor(color)
					->setXRay(2);
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
				output->cameFrom[current] = cameFrom[current];
				output->wentTo[cameFrom[current]] = current;
				current = cameFrom[current];
				output->add(current);
			}
			return output;
		}

		for(auto it = engine->chunkContainer->getNeighbors(entry.position); !it.atEnd(); ++it) {
			glm::ivec3 neighbor = it.value();
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

glm::uvec3 TileSet::getCameFrom(glm::uvec3 position) {
	if(this->cameFrom.find(position) != this->cameFrom.end()) {
		return this->cameFrom[position];
	}
	else {
		return glm::uvec3(-1, -1, -1);
	}
}

glm::uvec3 TileSet::getWentTo(glm::uvec3 position) {
	if(this->wentTo.find(position) != this->wentTo.end()) {
		return this->wentTo[position];
	}
	else {
		return glm::uvec3(-1, -1, -1);
	}
}

void es::defineTileSet() {
	esRegisterNamespace(engine->eggscript, "TileSet");
	esNamespaceInherit(engine->eggscript, "SimObject", "TileSet");

	esEntryType addArguments[2] = {ES_ENTRY_OBJECT, ES_ENTRY_MATRIX};
	esRegisterMethod(engine->eggscript, ES_ENTRY_NUMBER, es::TileSet__add, "TileSet", "add", 2, addArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_NUMBER, es::TileSet__remove, "TileSet", "remove", 2, addArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_NUMBER, es::TileSet__has, "TileSet", "has", 2, addArguments);

	esEntryType clearArguments[1] = {ES_ENTRY_OBJECT};
	esRegisterMethod(engine->eggscript, ES_ENTRY_EMPTY, es::TileSet__clear, "TileSet", "clear", 1, clearArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_NUMBER, es::TileSet__size, "TileSet", "size", 1, clearArguments);

	esRegisterMethod(engine->eggscript, ES_ENTRY_EMPTY, es::TileSet__showDots, "TileSet", "showDots", 1, clearArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_EMPTY, es::TileSet__showBorder, "TileSet", "showBorder", 1, clearArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_EMPTY, es::TileSet__hideBorder, "TileSet", "hideBorder", 1, clearArguments);

	esRegisterMethod(engine->eggscript, ES_ENTRY_MATRIX, es::TileSet__getCameFrom, "TileSet", "getCameFrom", 2, addArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_MATRIX, es::TileSet__getWentTo, "TileSet", "getWentTo", 2, addArguments);
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

esEntryPtr es::TileSet__size(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 1 && esCompareNamespaceToObject(args[0].objectData, "TileSet")) {
		return esCreateNumber(((TileSet*)args[0].objectData->objectWrapper->data)->size());
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
	else if(argc == 2 && esCompareNamespaceToObject(args[0].objectData, "TileSet") && args[1].matrixData->rows == 4 && args[1].matrixData->columns == 1) {
		((TileSet*)args[0].objectData->objectWrapper->data)->showBorder(
			glm::vec4(
				args[1].matrixData->data[0][0].numberData,
				args[1].matrixData->data[1][0].numberData,
				args[1].matrixData->data[2][0].numberData,
				args[1].matrixData->data[3][0].numberData
			)
		);
	}

	return nullptr;
}

esEntryPtr es::TileSet__hideBorder(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 1 && esCompareNamespaceToObject(args[0].objectData, "TileSet")) {
		((TileSet*)args[0].objectData->objectWrapper->data)->hideBorder();
	}

	return nullptr;
}

esEntryPtr es::TileSet__getCameFrom(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 2 && esCompareNamespaceToObject(args[0].objectData, "TileSet") && args[1].matrixData->rows == 3 && args[1].matrixData->columns == 1) {	
		glm::uvec3 position = ((TileSet*)args[0].objectData->objectWrapper->data)->getCameFrom(
			glm::uvec3(
				args[1].matrixData->data[0][0].numberData,
				args[1].matrixData->data[1][0].numberData,
				args[1].matrixData->data[2][0].numberData
			)
		);

		if(position == glm::uvec3(-1, -1, -1)) {
			return nullptr;
		}

		return esCreateVector(3, (double)position.x, (double)position.y, (double)position.z);
	}

	return nullptr;
}

esEntryPtr es::TileSet__getWentTo(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 1 && args[0].matrixData->rows == 3 && args[0].matrixData->columns == 1) {
		glm::uvec3 position = ((TileSet*)args[0].objectData->objectWrapper->data)->getWentTo(
			glm::uvec3(
				args[1].matrixData->data[0][0].numberData,
				args[1].matrixData->data[1][0].numberData,
				args[1].matrixData->data[2][0].numberData
			)
		);

		if(position == glm::uvec3(-1, -1, -1)) {
			return nullptr;
		}

		return esCreateVector(3, (double)position.x, (double)position.y, (double)position.z);
	}

	return nullptr;
}
