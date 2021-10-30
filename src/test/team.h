#pragma once

#include <vector>

#include "../basic/gameObject.h"

using namespace std;

namespace es {
	void defineTeam();
	esEntryPtr Team__size(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr Team__get(esEnginePtr esEngine, unsigned int argc, esEntry* args);
};

class Team: public GameObject {
	friend esEntryPtr es::Team__size(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	friend esEntryPtr es::Team__get(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	
	public:
		Team();

		void add(class Character* character);
	
	private:
		vector<class Character*> characters;
};
