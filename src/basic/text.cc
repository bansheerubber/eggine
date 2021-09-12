#include <glad/gl.h>
#include "text.h"

#include "ui.h"

GLuint Text::Shaders[2] = {GL_INVALID_INDEX, GL_INVALID_INDEX};
GLuint Text::Uniforms[3] = {GL_INVALID_INDEX, GL_INVALID_INDEX, GL_INVALID_INDEX};
GLuint Text::ShaderProgram = GL_INVALID_INDEX;

Text::Text(string family, int size) {
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

	glGenVertexArrays(1, &this->vao);
	glGenBuffers(1, &this->vbo);
	glBindVertexArray(this->vao);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Text::render(double deltaTime, RenderContext &context) {
	glUseProgram(Text::ShaderProgram);

	glUniformMatrix4fv(Text::Uniforms[2], 1, false, &context.ui->projectionMatrix[0][0]);

	glUniform3fv(Text::Uniforms[1], 1, &this->color[0]);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(Text::Uniforms[0], 0);
	
	glBindVertexArray(this->vao);

	float x = this->position.x, y = this->font->size + this->position.y, scale = 1.0f;

	// iterate through all characters
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

			// update VBO for each character
			float vertices[6][4] = {
					{ xpos,     ypos + h,   0.0f, 1.0f },            
					{ xpos,     ypos,       0.0f, 0.0f },
					{ xpos + w, ypos,       1.0f, 0.0f },

					{ xpos,     ypos + h,   0.0f, 1.0f },
					{ xpos + w, ypos,       1.0f, 0.0f },
					{ xpos + w, ypos + h,   1.0f, 1.0f }           
			};
			// render glyph texture over quad
			glBindTexture(GL_TEXTURE_2D, ch.texture);
			// update content of VBO memory
			glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			// render quad
			glDrawArrays(GL_TRIANGLES, 0, 6);
			// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
			x += (ch.advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
		}
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
