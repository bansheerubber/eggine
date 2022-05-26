#include "text.h"

#include "../engine/engine.h"
#include "../renderer/shader.h"
#include "ui.h"

render::Program* Text::Program = nullptr;

Text::Text(bool addToUiList) : RenderObject(false) {
	if(Text::Program == nullptr) {
		render::Shader* vertexShader = new render::Shader(&engine->renderWindow);
		vertexShader->load(getShaderSource("shaders/text.vert"), render::SHADER_VERTEX);

		render::Shader* fragmentShader = new render::Shader(&engine->renderWindow);
		fragmentShader->load(getShaderSource("shaders/text.frag"), render::SHADER_FRAGMENT);

		Text::Program = new render::Program(&engine->renderWindow);
		Text::Program->addShader(vertexShader);
		Text::Program->addShader(fragmentShader);
	}

	std::string filePrefix = "";
	#ifdef __switch__
	filePrefix = "romfs:/";
	#endif
	this->vertexBuffers[0] = new render::VertexBuffer(&engine->renderWindow, "textPositions");
	this->vertexBuffers[0]->setDynamicDraw(true);
	this->vertexBuffers[0]->setData(nullptr, sizeof(glm::vec2) * this->text.size() * 6, alignof(glm::vec2));

	this->vertexBuffers[1] = new render::VertexBuffer(&engine->renderWindow, "textUVs");
	this->vertexBuffers[1]->setDynamicDraw(true);
	this->vertexBuffers[1]->setData(nullptr, sizeof(glm::vec2) * this->text.size() * 6, alignof(glm::vec2));

	this->vertexAttributes = new render::VertexAttributes(&engine->renderWindow);
	this->vertexAttributes->addVertexAttribute(this->vertexBuffers[0], 0, 2, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec2), 0);
	this->vertexAttributes->addVertexAttribute(this->vertexBuffers[1], 1, 2, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec2), 0);

	if(addToUiList) {
		engine->addUIObject(this);
	}
}

Text::Text(std::string family, int size, bool addToUiList) : Text(addToUiList) {
	this->font = render::Font::GetFont(family, size);
}

Text::~Text() {
	
}

void Text::updateBuffers() {
	float x = 0, y = this->font->size, scale = 1.0f;
	glm::vec2 vertices[this->text.size() * 6];
	glm::vec2 uvs[this->text.size() * 6];

	// iterate through all characters
	uint64_t total = 0;
	for(uint64_t i = 0; i < this->text.size(); i++) {
		if(this->text[i] == '\n') {
			x = 0;
			y += this->font->size;
		}
		else {
			render::FontGlyph ch = this->font->characterToGlyph[(unsigned char)this->text[i]];

			float xpos = x + ch.left * scale;
			float ypos = y - ch.top;

			float w = ch.width * scale;
			float h = ch.height * scale;

			float u1 = ch.minUV.x;
			float v1 = ch.minUV.y;
			float u2 = ch.maxUV.x;
			float v2 = ch.maxUV.y;

			vertices[total][0] = xpos + w;
			vertices[total][1] = ypos;
			uvs[total][0] = u2;
			uvs[total][1] = v1;
			total++;

			vertices[total][0] = xpos;
			vertices[total][1] = ypos;
			uvs[total][0] = u1;
			uvs[total][1] = v1;
			total++;

			vertices[total][0] = xpos;
			vertices[total][1] = ypos + h;
			uvs[total][0] = u1;
			uvs[total][1] = v2;
			total++;

			vertices[total][0] = xpos + w;
			vertices[total][1] = ypos + h;
			uvs[total][0] = u2;
			uvs[total][1] = v2;
			total++;

			vertices[total][0] = xpos + w;
			vertices[total][1] = ypos;
			uvs[total][0] = u2;
			uvs[total][1] = v1;
			total++;

			vertices[total][0] = xpos;
			vertices[total][1] = ypos + h;
			uvs[total][0] = u1;
			uvs[total][1] = v2;
			total++;

			x += (ch.advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
		}
	}

	this->vertexBuffers[0]->reallocate();
	this->vertexBuffers[1]->reallocate();

	this->vertexBuffers[0]->setData(&vertices[0][0], sizeof(glm::vec2) * total, alignof(glm::vec2));
	this->vertexBuffers[1]->setData(&uvs[0][0], sizeof(glm::vec2) * total, alignof(glm::vec2));
}

void Text::setText(std::string text) {
	this->text = text;
}

std::string Text::getText() {
	return this->text;
}

void Text::setColor(glm::vec3 color) {
	this->color = color;
}

glm::vec3 Text::getColor() {
	return this->color;
}

void Text::setPosition(glm::vec2 position) {
	this->position = position;
}

glm::vec2 Text::getPosition() {
	return this->position;
}

void Text::setFont(render::Font* font) {
	this->font = font;
}

render::Font* Text::getFont() {
	return this->font;
}

void Text::render(double deltaTime, RenderContext &context) {
	if(this->oldText != this->text) {
		this->updateBuffers();
		this->oldText = this->text;
	}
	
	engine->renderWindow.getState(0).bindProgram(Text::Program);
	engine->renderWindow.getState(0).bindTexture("textTexture", this->font->texture);

	struct VertexBlock {
		glm::mat4 projection;
		glm::vec2 position;
	} vb;
	vb.projection = context.ui->getProjectionMatrix();
	vb.position = this->position;

	struct FragmentBlock {
		glm::vec3 color;
		float padding;
	} fb;
	fb.color = this->color;

	engine->renderWindow.getState(0).bindUniform("vertexBlock", &vb, sizeof(vb));
	engine->renderWindow.getState(0).bindUniform("fragmentBlock", &fb, sizeof(fb));

	engine->renderWindow.getState(0).bindVertexAttributes(this->vertexAttributes);

	engine->renderWindow.getState(0).draw(render::PRIMITIVE_TRIANGLES, 0, 6 * this->text.size(), 0, 1);
}
