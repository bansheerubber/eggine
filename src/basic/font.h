#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include <glfw/glfw3.h>
#include <tsl/robin_map.h>
#include <string>

#include "gameObject.h"

using namespace std;

struct FontGlyph {
	GLuint texture;
	unsigned int width;
	unsigned int height;
	int left;
	int top;
	long int advance;
};

class Font : public GameObject {
	friend class Text;
	
	public:
		Font(string fileName, int size);
		string fileName;

		static tsl::robin_map<string, tsl::robin_map<int, Font*>> Fonts;
		static Font* GetFont(string &family, int size);

		int size;
	
	protected:
		FT_Face face;
		GLuint textures[128];
		FontGlyph characterToGlyph[128];
};
