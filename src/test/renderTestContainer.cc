#include <glad/gl.h>
#include "renderTestContainer.h"

#include <random>

RenderTestContainer::RenderTestContainer() {
	glGenBuffers(2, this->vertexBufferObjects);
	glGenVertexArrays(1, &this->vertexArrayObject);
	glBindVertexArray(this->vertexArrayObject);

	for(size_t i = 0; i < 1000000; i++) {
		this->offsets[i][0] = ((double) rand() / (RAND_MAX)) * 2 - 1;
		this->offsets[i][1] = ((double) rand() / (RAND_MAX)) * 2 - 1;
	}
	
	// load vertices
	{
		glBindBuffer(GL_ARRAY_BUFFER, this->vertexBufferObjects[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(this->vertices), this->vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);
	}

	// load offsets
	{
		glBindBuffer(GL_ARRAY_BUFFER, this->vertexBufferObjects[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 1000000, &this->offsets[0], GL_STATIC_DRAW);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
		glVertexAttribDivisor(1, 1);
		glEnableVertexAttribArray(1);
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

	RenderObject::LinkProgram(&this->shaderProgram, this->shaders, 2);
}

void RenderTestContainer::render(double deltaTime) {
	glUseProgram(this->shaderProgram);
	glBindVertexArray(this->vertexArrayObject);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 3, 100000);
}
