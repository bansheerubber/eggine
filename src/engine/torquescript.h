#pragma once

#include <torquescript/ts.h>

namespace ts {
	void torquescriptDefinitions();
	tsEntryPtr addKeybind(tsEnginePtr engine, unsigned int argc, tsEntryPtr args);
}
