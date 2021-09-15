#include <glad/gl.h>
#include "renderTestContainer.h"

#include <random>

#include "../basic/camera.h"
#include "../util/doubleDimension.h"
#include "tileMath.h"

PNGImage* RenderTestContainer::Image = nullptr;
GLuint RenderTestContainer::Texture = 0;

RenderTestContainer::RenderTestContainer(glm::vec2 position) {
	this->position = position;

	glGenBuffers(4, this->vertexBufferObjects);
	glGenVertexArrays(1, &this->vertexArrayObject);
	glBindVertexArray(this->vertexArrayObject);

	this->offsets = new glm::vec2[Size * Size * this->height];
	this->textureIndices = new int[Size * Size * this->height];

	for(unsigned i = 0; i < Size * Size * this->height; i++) {
		this->offsets[i] = glm::vec2(0, 0);
		this->textureIndices[i] = 2;
	}

	for(unsigned z = 0; z < this->height; z++) {
		for(unsigned i = 0; i < Size * Size; i++) {
			glm::ivec2 coordinate = tilemath::indexToCoordinate(i, Size);
			this->offsets[i + z * Size * Size] = tilemath::tileToScreen(glm::vec3(coordinate, z)) + tilemath::tileToScreen(glm::vec3((float)Size * this->position, 0.0));
			this->textureIndices[i + z * Size * Size] = 2;
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
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * RenderTestContainer::Size * RenderTestContainer::Size * this->height, &this->offsets[0], GL_STATIC_DRAW);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribDivisor(2, 1);
		glEnableVertexAttribArray(2);
	}

	// load texture indices
	{
		glBindBuffer(GL_ARRAY_BUFFER, this->vertexBufferObjects[3]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(int) * RenderTestContainer::Size * RenderTestContainer::Size * this->height, this->textureIndices, GL_STATIC_DRAW);

		glVertexAttribIPointer(3, 1, GL_INT, 0, 0);
		glVertexAttribDivisor(3, 1);
		glEnableVertexAttribArray(3);
	}

	// load textures
	if(Image == nullptr) {
		Image = new PNGImage("data/spritesheet.png");
		glGenTextures(1, &Texture);
		glBindTexture(GL_TEXTURE_2D, Texture);

		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			Image->getFormat(),
			Image->width,
			Image->height,
			0,
			Image->getFormat(),
			Image->getType(),
			Image->image
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
	glBindTexture(GL_TEXTURE_2D, Texture);
	glUniform1i(this->uniforms[1], 0); // bind texture

	glUniformMatrix4fv(this->uniforms[0], 1, false, &context.camera->projectionMatrix[0][0]);

	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, RenderTestContainer::Size * RenderTestContainer::Size * this->height);
}
