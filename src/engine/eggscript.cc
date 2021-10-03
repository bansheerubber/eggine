#include "eggscript.h"

#include "engine.h"

void es::eggscriptDefinitions() {
	esEntryType addKeybindArguments[3] = {ES_ENTRY_STRING, ES_ENTRY_STRING, ES_ENTRY_STRING};
	esRegisterFunction(engine->eggscript, ES_ENTRY_INVALID, &addKeybind, "addKeybind", 3, addKeybindArguments);
}

esEntryPtr es::addKeybind(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 3) {
		engine->registerTSKeybindCallback(args[0].stringData, args[1].stringData, args[2].stringData);
	}
	return nullptr;
}
