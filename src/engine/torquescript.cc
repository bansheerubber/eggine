#include "torquescript.h"

#include "engine.h"

void ts::torquescriptDefinitions() {
	tsEntryType addKeybindArguments[3] = {TS_ENTRY_STRING, TS_ENTRY_STRING, TS_ENTRY_STRING};
	tsRegisterFunction(engine->torquescript, TS_ENTRY_INVALID, &addKeybind, "addKeybind", 3, addKeybindArguments);
}

tsEntryPtr ts::addKeybind(tsEnginePtr tsEngine, unsigned int argc, tsEntryPtr args) {
	if(argc == 3) {
		engine->registerTSKeybindCallback(args[0].stringData, args[1].stringData, args[2].stringData);
	}
	return nullptr;
}
