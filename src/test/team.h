#pragma once

#include <vector>

#include "../basic/gameObject.h"
#include "../util/sortedArray.h"

namespace es {
	void defineTeam();
	esEntryPtr Team__size(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	esEntryPtr Team__get(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	esEntryPtr Team__has(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	esEntryPtr Team__setName(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	esEntryPtr Team__getName(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	esEntryPtr getTeamCount(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	esEntryPtr getTeam(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
};

int teamUnitSort(class Unit** character1, class Unit** character2);

class Team: public GameObject {
	friend esEntryPtr es::Team__size(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	friend esEntryPtr es::Team__get(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	friend esEntryPtr es::Team__has(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	
	public:
		Team();
		~Team();

		// ## game_object_definitions Team

		std::string name;

		void add(class Unit* character);
		void remove(class Unit* character);

		static std::vector<Team*> Teams;
	
	private:
		SortedArray<class Unit*> units = SortedArray<class Unit*>(teamUnitSort, nullptr, nullptr);
};
