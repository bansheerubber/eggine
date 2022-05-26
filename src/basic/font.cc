#include "font.h"

#include <filesystem>

#include "../engine/console.h"
#include "../engine/engine.h"

tsl::robin_map<std::string, tsl::robin_map<int, render::Font*>> render::Font::Fonts;

render::Font* render::Font::GetFont(std::string family, int size) {
	if(Font::Fonts[family][size] == nullptr) {
		// TODO load this from the carton
		#if defined(__switch__) || defined(_WIN32)
		return new Font(engine->filePrefix + "arial.ttf", size);
		#else
		return new Font("/usr/share/fonts/TTF/arial.ttf", size);
		#endif
	}
	else {
		return Font::Fonts[family][size];
	}
}

render::Font::Font(std::string fileName, int size) {
	this->fileName = fileName;
	this->size = size;

	if(!std::filesystem::exists(fileName)) {
		console::error("could not load font %s\n", fileName.c_str());
		exit(1);
	}

	FT_New_Face(engine->ft, fileName.c_str(), 0, &this->face);
	FT_Set_Pixel_Sizes(this->face, 0, size);

	uint64_t amount = 128;

	std::string name(this->face->family_name);
	Font::Fonts[name][size] = this;

	for(uint64_t i = 0; i < 256 * 256; i++) {
		this->atlas[i] = 0;
	}

	int padding = 1;
	int atlasX = padding, atlasY = padding; // keep track of where we are during generation of the atlas
	for(unsigned char c = 0; c < amount; c++) {
		if(FT_Load_Char(this->face, c, FT_LOAD_RENDER)) {
			console::error("could not load gylph %c from %s\n", c, this->fileName.c_str());
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

		for(uint64_t x = 0; x < this->face->glyph->bitmap.width; x++) {
			for(uint64_t y = 0; y < this->face->glyph->bitmap.rows; y++) {
				uint64_t index = y * this->face->glyph->bitmap.pitch + x;
				this->atlas[(y + atlasY) * 256 + (x + atlasX) % 256] = this->face->glyph->bitmap.buffer[index];
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

		if(c == 'z' || c == 'w' || c == 'v' || c == 'x') {
			this->xHeight += this->face->glyph->bitmap_top;
		}

		if(this->characterToGlyph[c].top > this->ascent) {
			this->ascent = this->characterToGlyph[c].top;
		}

		if((this->face->glyph->metrics.height >> 6) - this->characterToGlyph[c].top > this->descent) {
			this->descent = (this->face->glyph->metrics.height >> 6) - this->characterToGlyph[c].top;
		}
	}

	this->xHeight /= 4;

	this->texture = new render::Texture(&engine->renderWindow);
	this->texture->setWrap(render::TEXTURE_WRAP_CLAMP_TO_EDGE, render::TEXTURE_WRAP_CLAMP_TO_EDGE);
	this->texture->setFilters(render::TEXTURE_FILTER_LINEAR, render::TEXTURE_FILTER_LINEAR);

	this->texture->load((unsigned char*)this->atlas, 256 * 256, 256, 256, 8, 1);

	FT_Done_Face(face);
}

render::FontInfo render::Font::getInfo() {
	return {
		this->size,
		this->ascent,
		this->descent,
		this->xHeight,
	};
}

render::FontGlyph render::Font::getGlyph(unsigned char character) {
	return this->characterToGlyph[character];
}
