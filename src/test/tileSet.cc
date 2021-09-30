#include "tileSet.h"

#include "../engine/engine.h"

TileSet::TileSet() {
	this->reference = tsCreateObject(engine->torquescript, "TileSet", this);
}

TileSet::~TileSet() {
	tsDeleteObject(this->reference);
}

bool TileSet::add(glm::uvec2 position) {
	bool alreadyHas = this->has(position);
	if(!alreadyHas) {
		this->set.insert(this->index(position));
	}
	return alreadyHas;
}

bool TileSet::remove(glm::uvec2 position) {
	bool alreadyHas = this->has(position);
	if(alreadyHas) {
		this->set.erase(this->index(position));
	}
	return alreadyHas;
}

bool TileSet::has(glm::uvec2 position) {
	return this->set.find(this->index(position)) != this->set.end();
}

void TileSet::clear() {
	this->set.clear();
}

unsigned int TileSet::index(glm::uvec2 &position) {
	return ((position.x + position.y) * (position.x + position.y + 1)) / 2 + position.y;
}

void ts::defineTileSet() {
	tsRegisterNamespace(engine->torquescript, "TileSet");
	tsNamespaceInherit(engine->torquescript, "SimObject", "TileSet");

	tsEntryType addArguments[3] = {TS_ENTRY_OBJECT, TS_ENTRY_NUMBER, TS_ENTRY_NUMBER};
	tsRegisterMethod(engine->torquescript, TS_ENTRY_NUMBER, ts::TileSet__add, "TileSet", "add", 3, addArguments);
	tsRegisterMethod(engine->torquescript, TS_ENTRY_NUMBER, ts::TileSet__remove, "TileSet", "remove", 3, addArguments);
	tsRegisterMethod(engine->torquescript, TS_ENTRY_NUMBER, ts::TileSet__has, "TileSet", "has", 3, addArguments);

	tsEntryType clearArguments[1] = {TS_ENTRY_OBJECT};
	tsRegisterMethod(engine->torquescript, TS_ENTRY_INVALID, ts::TileSet__clear, "TileSet", "clear", 1, clearArguments);
}

tsEntryPtr ts::TileSet__add(tsEnginePtr tsEngine, unsigned int argc, tsEntry* args) {
	if(argc == 3 && tsCompareNamespaceToObject(args[0].objectData, "TileSet")) {
		tsEntryPtr entry = new tsEntry();
		entry->type = TS_ENTRY_NUMBER;
		entry->numberData = ((TileSet*)args[0].objectData->objectWrapper->data)->add(glm::uvec2((int)args[1].numberData, (int)args[2].numberData));
		return entry;
	}

	return nullptr;
}

tsEntryPtr ts::TileSet__remove(tsEnginePtr tsEngine, unsigned int argc, tsEntry* args) {
	if(argc == 3 && tsCompareNamespaceToObject(args[0].objectData, "TileSet")) {
		tsEntryPtr entry = new tsEntry();
		entry->type = TS_ENTRY_NUMBER;
		entry->numberData = ((TileSet*)args[0].objectData->objectWrapper->data)->remove(glm::uvec2((int)args[1].numberData, (int)args[2].numberData));
		return entry;
	}

	return nullptr;
}

tsEntryPtr ts::TileSet__has(tsEnginePtr tsEngine, unsigned int argc, tsEntry* args) {
	if(argc == 3 && tsCompareNamespaceToObject(args[0].objectData, "TileSet")) {
		tsEntryPtr entry = new tsEntry();
		entry->type = TS_ENTRY_NUMBER;
		entry->numberData = ((TileSet*)args[0].objectData->objectWrapper->data)->has(glm::uvec2((int)args[1].numberData, (int)args[2].numberData));
		return entry;
	}

	return nullptr;
}

tsEntryPtr ts::TileSet__clear(tsEnginePtr tsEngine, unsigned int argc, tsEntry* args) {
	if(argc == 1 && tsCompareNamespaceToObject(args[0].objectData, "TileSet")) {
		((TileSet*)args[0].objectData->objectWrapper->data)->clear();
	}

	return nullptr;
}
