#include "../helpers.h"
#include GLAD_HEADER

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
	glGenTextures(1, &this->texture);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	string name(this->face->family_name);
	Font::Fonts[name][size] = this;

	for(size_t x = 0; x < 256; x++) {
		for(size_t y = 0; y < 256; y++) {
			this->atlas[x][y] = 0;
		}	
	}

	int padding = 1;
	int atlasX = padding, atlasY = padding; // keep track of where we are during generation of the atlas
	for(char c = 0; c < amount; c++) {
		if(FT_Load_Char(this->face, c, FT_LOAD_RENDER)) {
			printf("could not load gylph %c from %s\n", c, this->fileName.c_str());
			continue;
		}

		if(atlasX + this->face->glyph->bitmap.width + padding >= 256) {
			atlasX = padding;
			atlasY += size + padding;
		}

		glm::vec2 minUV(
			(float)atlasX / 256.0f,
			(float)atlasY / 256.0f
		);
		glm::vec2 maxUV(
			(float)(atlasX + this->face->glyph->bitmap.width) / 256.0f,
			(float)(atlasY + this->face->glyph->bitmap.rows) / 256.0f
		);

		for(size_t x = 0; x < this->face->glyph->bitmap.width; x++) {
			for(size_t y = 0; y < this->face->glyph->bitmap.rows; y++) {
				size_t index = y * this->face->glyph->bitmap.pitch + x;
				this->atlas[y + atlasY][x + atlasX] = this->face->glyph->bitmap.buffer[index];
			}
		}
		atlasX += this->face->glyph->bitmap.width + padding;

		this->characterToGlyph[c] = FontGlyph {
			width: this->face->glyph->bitmap.width,
			height: this->face->glyph->bitmap.rows,
			left: this->face->glyph->bitmap_left,
			top: this->face->glyph->bitmap_top,
			advance: this->face->glyph->advance.x,
			minUV: minUV,
			maxUV: maxUV,
		};
	}

	glBindTexture(GL_TEXTURE_2D, this->texture);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RED,
		256,
		256,
		0,
		GL_RED,
		GL_UNSIGNED_BYTE,
		this->atlas
	);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // reset alignment

	FT_Done_Face(face);
}
