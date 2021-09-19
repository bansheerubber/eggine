#pragma once

#include <string>
#include <vector>

using namespace std;

#define EGGINE_DEBUG 1

class Debug {
	friend class Engine;

	public:
		void addInfoMessage(string message);
	
	private:
		string getInfoText();
		void clearInfoMessages();
		vector<string> infoMessages;
};
