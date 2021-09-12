#include <glad/gl.h>
#include "font.h"

#include "../engine/engine.h"

tsl::robin_map<string, tsl::robin_map<int, Font*>> Font::Fonts;

Font* Font::GetFont(string &family, int size) {
	if(Font::Fonts[family][size] == nullptr) {
		return new Font("/usr/share/fonts/TTF/arial.ttf", size);
	}
	else {
		return Font::Fonts[family][size];
	}
}

Font::Font(string fileName, int size) {
	this->fileName = fileName;
	this->size = size;

	FT_New_Face(engine->ft, fileName.c_str(), 0, &this->face);
	FT_Set_Pixel_Sizes(this->face, 0, size);

	size_t amount = 128;
	glGenTextures(amount, this->textures);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	string name(this->face->family_name);
	Font::Fonts[name][size] = this;

	for(char c = 0; c < amount; c++) {
		if(FT_Load_Char(this->face, c, FT_LOAD_RENDER)) {
			printf("could not load gylph %c from %s\n", c, this->fileName.c_str());
			continue;
		}

		glBindTexture(GL_TEXTURE_2D, this->textures[c]);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		this->characterToGlyph[c] = FontGlyph {
			texture: this->textures[c],
			width: this->face->glyph->bitmap.width,
			height: this->face->glyph->bitmap.rows,
			left: this->face->glyph->bitmap_left,
			top: this->face->glyph->bitmap_top,
			advance: this->face->glyph->advance.x,
		};
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // reset alignment

	FT_Done_Face(face);
}
