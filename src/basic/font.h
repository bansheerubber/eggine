#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/vec2.hpp>
#include <tsl/robin_map.h>
#include <string>

#include "gameObject.h"
#include "../renderer/texture.h"

class Text;

namespace render {
	struct FontGlyph {
		unsigned int width;
		unsigned int height;
		int left;
		int top;
		int64_t advance;
		glm::vec2 minUV;
		glm::vec2 maxUV;
	};

	class Font : public GameObject {
		friend Text;
		
		public:
			Font(std::string fileName, int size);
			std::string fileName;

			static tsl::robin_map<std::string, tsl::robin_map<int, Font*>> Fonts;
			static Font* GetFont(std::string family, int size);

			int size = 0;
			char atlas[256 * 256];
			int ascent = 0;
			int descent = 0;
			int x_height = 0;
			FontGlyph characterToGlyph[128];
		
		protected:
			FT_Face face;
			render::Texture* texture = nullptr;
	};
};