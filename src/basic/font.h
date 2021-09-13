#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include <glfw/glfw3.h>
#include <glm/vec2.hpp>
#include <tsl/robin_map.h>
#include <string>

#include "gameObject.h"

using namespace std;

struct FontGlyph {
	unsigned int width;
	unsigned int height;
	int left;
	int top;
	long int advance;
	glm::vec2 minUV;
	glm::vec2 maxUV;
};

class Font : public GameObject {
	friend class Text;
	
	public:
		Font(string fileName, int size);
		string fileName;

		static tsl::robin_map<string, tsl::robin_map<int, Font*>> Fonts;
		static Font* GetFont(string &family, int size);

		int size;
		char atlas[256][256];
	
	protected:
		FT_Face face;
		GLuint texture;
		FontGlyph characterToGlyph[128];
};
