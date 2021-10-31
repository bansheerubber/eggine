#pragma once

#include <vector>

#include "../basic/gameObject.h"
#include "../util/sortedArray.h"

using namespace std;

namespace es {
	void defineTeam();
	esEntryPtr Team__size(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr Team__get(esEnginePtr esEngine, unsigned int argc, esEntry* args);
};

int teamCharacterSort(class Character** character1, class Character** character2);

class Team: public GameObject {
	friend esEntryPtr es::Team__size(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	friend esEntryPtr es::Team__get(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	
	public:
		Team();

		void add(class Character* character);
	
	private:
		SortedArray<class Character*> characters = SortedArray<class Character*>(teamCharacterSort, nullptr, nullptr);
};
