#pragma once

#include <eggscript/egg.h>

namespace es {
	void eggscriptDefinitions();
	esEntryPtr exec(esEnginePtr engine, unsigned int argc, esEntryPtr args);
	esEntryPtr addKeybind(esEnginePtr engine, unsigned int argc, esEntryPtr args);
	esEntryPtr SimObject__addKeybind(esEnginePtr engine, unsigned int argc, esEntryPtr args);
	esEntryPtr probeGarbage(esEnginePtr engine, unsigned int argc, esEntryPtr args);
}
