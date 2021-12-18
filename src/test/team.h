#pragma once

#include <vector>

#include "../basic/gameObject.h"
#include "../util/sortedArray.h"

using namespace std;

namespace es {
	void defineTeam();
	esEntryPtr Team__size(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr Team__get(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr Team__has(esEnginePtr esEngine, unsigned int argc, esEntry* args);
};

int teamUnitSort(class Unit** character1, class Unit** character2);

class Team: public GameObject {
	friend esEntryPtr es::Team__size(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	friend esEntryPtr es::Team__get(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	friend esEntryPtr es::Team__has(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	
	public:
		Team();

		// ## game_object_definitions Team

		void add(class Unit* character);
		void remove(class Unit* character);
	
	private:
		SortedArray<class Unit*> units = SortedArray<class Unit*>(teamUnitSort, nullptr, nullptr);
};
