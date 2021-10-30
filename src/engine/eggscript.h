#pragma once

#include <eggscript/egg.h>

namespace es {
	void eggscriptDefinitions();
	esEntryPtr addKeybind(esEnginePtr engine, unsigned int argc, esEntryPtr args);
	esEntryPtr SimObject__addKeybind(esEnginePtr engine, unsigned int argc, esEntryPtr args);
}
