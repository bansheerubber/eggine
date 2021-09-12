#include <glad/gl.h>
#include "renderTestContainer.h"

#include <random>

#include "../basic/camera.h"

RenderTestContainer::RenderTestContainer() {
	glGenBuffers(3, this->vertexBufferObjects);
	glGenVertexArrays(1, &this->vertexArrayObject);
	glBindVertexArray(this->vertexArrayObject);

	int width = 400;
	int height = 400;
	int total = 0;
	for(int d = height - 2; d >= 0; d--) {
		for(int x = width - 1, y = height - 2 - d; y >= 0; x--, y--) {
			this->offsets[total][0] = x * 0.06f / 2 + y * 0.06f / 2;
			this->offsets[total][1] = -(x * -0.06f / 4 + y * 0.06f / 4);
			total++;
		}
	}

	for(int d = width - 1; d >= 0; d--) {
		for(int x = d, y = height - 1; x >= 0; x--, y--) {
			this->offsets[total][0] = x * 0.06f / 2 + y * 0.06f / 2;
			this->offsets[total][1] = -(x * -0.06f / 4 + y * 0.06f / 4);
			total++;
		}
	}
	
	// load vertices
	{
		glBindBuffer(GL_ARRAY_BUFFER, this->vertexBufferObjects[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(this->vertices), this->vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);
	}

	// load uvs
	{
		glBindBuffer(GL_ARRAY_BUFFER, this->vertexBufferObjects[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(this->uvs), this->uvs, GL_STATIC_DRAW);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);
	}

	// load offsets
	{
		glBindBuffer(GL_ARRAY_BUFFER, this->vertexBufferObjects[2]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 160000, &this->offsets[0], GL_STATIC_DRAW);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribDivisor(2, 1);
		glEnableVertexAttribArray(2);
	}

	// load textures
	{
		glGenTextures(1, &this->texture);
		glBindTexture(GL_TEXTURE_2D, this->texture);

		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			this->image.getFormat(),
			this->image.width,
			this->image.height,
			0,
			this->image.getFormat(),
			this->image.getType(),
			this->image.image
		);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	glBindVertexArray(0); // turn off vertex array object

	RenderObject::CompileShader(
		GL_VERTEX_SHADER,
		&this->shaders[0],
		#include "shaders/test.vert"
	);

	RenderObject::CompileShader(
		GL_FRAGMENT_SHADER,
		&this->shaders[1],
		#include "shaders/test.frag"
	);

	if(RenderObject::LinkProgram(&this->shaderProgram, this->shaders, 2)) {
		this->uniforms[0] = glGetUniformLocation(this->shaderProgram, "projection");
		this->uniforms[1] = glGetUniformLocation(this->shaderProgram, "spriteTexture");
	}
}

void RenderTestContainer::render(double deltaTime, RenderContext &context) {
	glUseProgram(this->shaderProgram);
	glBindVertexArray(this->vertexArrayObject);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->texture);
	glUniform1i(this->uniforms[1], 0); // bind texture

	glUniformMatrix4fv(this->uniforms[0], 1, false, &context.camera->projectionMatrix[0][0]);

	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 160000);
}
