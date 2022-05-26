#pragma once

#include "../engine/developer.h"

#ifdef EGGINE_DEVELOPER_MODE
#include <vector>

#include "../util/png.h"

struct ConsoleEntry {
	int level;
	std::string contents;
};

int developerPrint(const char* buffer, ...);
int developerWarning(const char* buffer, ...);
int developerError(const char* buffer, ...);

int vDeveloperPrint(const char* buffer, va_list args);
int vDeveloperWarning(const char* buffer, va_list args);
int vDeveloperError(const char* buffer, va_list args);

namespace render {
	class Texture;
};

class DeveloperGui {
	public:
		DeveloperGui();
		
		void render();
		void prerender();
		void addEntry(ConsoleEntry entry);
		void focusConsole();
		void setSpritesheet(png spritesheet);
		png getSpritesheet();
		void addSpritesheetTexture(render::Texture* texture);
	
	private:
		std::vector<std::string> history;
		int historyPosition = 0;
		std::string incompleteCommand = "";
		bool jumpToBottom = false;
		bool consoleFullSize = false; // whether or not the console takes up the full extent of its scroll
		bool _focusConsole = false;
		png spritesheet;
		std::vector<ConsoleEntry> console;
		std::vector<render::Texture*> spritesheetTextures;

		int consoleCallback(ImGuiInputTextCallbackData* data);
};
#endif
