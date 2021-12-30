#pragma once

#include "../engine/developer.h"

#ifdef EGGINE_DEVELOPER_MODE
#include <vector>

#include "../util/png.h"
#include "../renderer/texture.h"

struct ConsoleEntry {
	int level;
	std::string contents;
};

class DeveloperGui {
	public:
		DeveloperGui();

		png spritesheet;
		std::vector<render::Texture*> spritesheetImages;

		std::vector<ConsoleEntry> console;

		bool focusConsole = false;
		
		void render();
		void prerender();
	
	private:
		std::vector<std::string> history;
		int historyPosition = 0;
		std::string incompleteCommand = "";
		int consoleCallback(ImGuiInputTextCallbackData* data);
};
#endif
