#pragma once

#include "../engine/developer.h"

#ifdef EGGINE_DEVELOPER_MODE
#include <vector>

#include "../util/png.h"
#include "../renderer/texture.h"

using namespace std;

class DeveloperGui {
	public:
		png spritesheet;
		vector<render::Texture*> spritesheetImages;
		
		void render();
		void prerender();
};
#endif
