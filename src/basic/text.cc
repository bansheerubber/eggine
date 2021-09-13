#include <glad/gl.h>
#include "text.h"

#include "../engine/engine.h"
#include "ui.h"

GLuint Text::Shaders[2] = {GL_INVALID_INDEX, GL_INVALID_INDEX};
GLuint Text::Uniforms[3] = {GL_INVALID_INDEX, GL_INVALID_INDEX, GL_INVALID_INDEX};
GLuint Text::ShaderProgram = GL_INVALID_INDEX;

Text::Text(string family, int size) : RenderObject(false) {
	this->font = Font::GetFont(family, size);

	RenderObject::CompileShader(
		GL_VERTEX_SHADER,
		&Text::Shaders[0],
		#include "shaders/text.vert"
	);

	RenderObject::CompileShader(
		GL_FRAGMENT_SHADER,
		&Text::Shaders[1],
		#include "shaders/text.frag"
	);

	if(RenderObject::LinkProgram(&Text::ShaderProgram, Text::Shaders, 2)) {
		Text::Uniforms[0] = glGetUniformLocation(Text::ShaderProgram, "textTexture");
		Text::Uniforms[1] = glGetUniformLocation(Text::ShaderProgram, "textColor");
		Text::Uniforms[2] = glGetUniformLocation(Text::ShaderProgram, "projection");
	}

	// new code
	glGenBuffers(2, this->vertexBufferObjects);
	glGenVertexArrays(1, &this->vertexArrayObject);
	glBindVertexArray(this->vertexArrayObject);

	// create vertices buffer
	{
		glBindBuffer(GL_ARRAY_BUFFER, this->vertexBufferObjects[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * this->text.size() * 6, NULL, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);
	}

	// create uv buffer
	{
		glBindBuffer(GL_ARRAY_BUFFER, this->vertexBufferObjects[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * this->text.size() * 6, NULL, GL_STATIC_DRAW);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
		glEnableVertexAttribArray(1);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	engine->addUIObject(this);
}

void Text::updateBuffers() {
	float x = this->position.x, y = this->font->size + this->position.y, scale = 1.0f;

	glm::vec2 vertices[this->text.size() * 6];
	glm::vec2 uvs[this->text.size() * 6];

	// iterate through all characters
	size_t total = 0;
	for(size_t i = 0; i < this->text.size(); i++) {
		if(this->text[i] == '\n') {
			x = this->position.x;
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

			vertices[total][0] = xpos;
			vertices[total][1] = ypos + h;
			uvs[total][0] = u1;
			uvs[total][1] = v2;
			total++;

			vertices[total][0] = xpos;
			vertices[total][1] = ypos;
			uvs[total][0] = u1;
			uvs[total][1] = v1;
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

			vertices[total][0] = xpos + w;
			vertices[total][1] = ypos;
			uvs[total][0] = u2;
			uvs[total][1] = v1;
			total++;

			vertices[total][0] = xpos + w;
			vertices[total][1] = ypos + h;
			uvs[total][0] = u2;
			uvs[total][1] = v2;
			total++;

			x += (ch.advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
		}
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, this->vertexBufferObjects[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * this->text.size() * 6, NULL, GL_DYNAMIC_DRAW); // orphan buffer
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec2) * this->text.size() * 6, &vertices[0][0]);

	glBindBuffer(GL_ARRAY_BUFFER, this->vertexBufferObjects[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * this->text.size() * 6, NULL, GL_DYNAMIC_DRAW); // orphan buffer
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec2) * this->text.size() * 6, &uvs[0][0]);
}

void Text::setText(string text) {
	this->text = text;
	this->updateBuffers();
}

string Text::getText() {
	return this->text;
}

void Text::render(double deltaTime, RenderContext &context) {
	glUseProgram(Text::ShaderProgram);
	glBindVertexArray(this->vertexArrayObject);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->font->texture);

	glUniformMatrix4fv(Text::Uniforms[2], 1, false, &context.ui->projectionMatrix[0][0]);
	glUniform3fv(Text::Uniforms[1], 1, &this->color[0]);
	glUniform1i(Text::Uniforms[0], 0);

	glDrawArrays(GL_TRIANGLES, 0, 6 * this->text.size());
}
