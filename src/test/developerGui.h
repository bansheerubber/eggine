#pragma once

#include "../engine/developer.h"

#ifdef EGGINE_DEVELOPER_MODE
#include <vector>

#include "../util/png.h"
#include "../renderer/texture.h"

using namespace std;

struct ConsoleEntry {
	int level;
	string contents;
};

class DeveloperGui {
	public:
		DeveloperGui();

		png spritesheet;
		vector<render::Texture*> spritesheetImages;

		vector<ConsoleEntry> console;

		bool focusConsole = false;
		
		void render();
		void prerender();
	
	private:
		vector<string> history;
		int historyPosition = 0;
		string incompleteCommand = "";
		int consoleCallback(ImGuiInputTextCallbackData* data);
};
#endif
