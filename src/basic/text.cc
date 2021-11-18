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

	glm::vec2 triangleVertices[] = {
		glm::vec2(-0.5, 1.0),
		glm::vec2(-0.5, -1.0),
		glm::vec2(0.5, 1.0),
		glm::vec2(0.5, -1.0),
	};

	glm::vec2 triangleUVs[] = {
		glm::vec2(0.0, 0.0),
		glm::vec2(0.0, 1.0),
		glm::vec2(1.0, 0.0),
		glm::vec2(1.0, 1.0),
	};

	string filePrefix = "";
	#ifdef __switch__
	filePrefix = "romfs:/";
	#endif
	this->vertexBuffers[0] = new render::VertexBuffer(&engine->renderWindow);
	this->vertexBuffers[0]->setDynamicDraw(true);
	this->vertexBuffers[0]->setData(nullptr, sizeof(glm::vec2) * this->text.size() * 6, alignof(glm::vec2));

	this->vertexBuffers[1] = new render::VertexBuffer(&engine->renderWindow);
	this->vertexBuffers[1]->setDynamicDraw(true);
	this->vertexBuffers[1]->setData(nullptr, sizeof(glm::vec2) * this->text.size() * 6, alignof(glm::vec2));

	this->vertexAttributes = new render::VertexAttributes(&engine->renderWindow);
	this->vertexAttributes->addVertexAttribute(this->vertexBuffers[0], 0, 2, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec2), 0);
	this->vertexAttributes->addVertexAttribute(this->vertexBuffers[1], 1, 2, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec2), 0);

	if(addToUiList) {
		engine->addUIObject(this);
	}
}

Text::Text(string family, int size, bool addToUiList) : Text(addToUiList) {
	this->font = Font::GetFont(family, size);
}

Text::~Text() {
	
}

void Text::updateBuffers() {
	float x = 0, y = this->font->size, scale = 1.0f;
	glm::vec2 vertices[this->text.size() * 6];
	glm::vec2 uvs[this->text.size() * 6];

	// iterate through all characters
	size_t total = 0;
	for(size_t i = 0; i < this->text.size(); i++) {
		if(this->text[i] == '\n') {
			x = 0;
			y += this->font->size;
		}
		else {
			FontGlyph ch = this->font->characterToGlyph[this->text[i]];

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

	this->vertexBuffers[0]->setData(&vertices[0][0], sizeof(glm::vec2) * this->text.size() * 6, alignof(glm::vec2));
	this->vertexBuffers[1]->setData(&uvs[0][0], sizeof(glm::vec2) * this->text.size() * 6, alignof(glm::vec2));
}

void Text::setText(string text) {
	this->text = text;
	this->updateBuffers();
}

string Text::getText() {
	return this->text;
}

void Text::render(double deltaTime, RenderContext &context) {
	Text::Program->bind();
	this->font->texture->bind(0);
	Text::Program->bindTexture("textTexture", 0);

	struct VertexBlock {
		glm::mat4 projection;
		glm::vec2 position;
	} vb;
	vb.projection = context.ui->projectionMatrix;
	vb.position = this->position;

	struct FragmentBlock {
		glm::vec3 color;
		float padding;
	} fb;
	fb.color = this->color;

	Text::Program->bindUniform("vertexBlock", &vb, sizeof(vb));
	Text::Program->bindUniform("fragmentBlock", &fb, sizeof(fb));

	this->vertexAttributes->bind();

	engine->renderWindow.draw(render::PRIMITIVE_TRIANGLES, 0, 6 * this->text.size(), 0, 1);
}
