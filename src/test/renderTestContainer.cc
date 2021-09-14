#include <glad/gl.h>
#include "renderTestContainer.h"

#include <random>

#include "../basic/camera.h"
#include "tileMath.h"

RenderTestContainer::RenderTestContainer(bool houseLayer) {
	glGenBuffers(4, this->vertexBufferObjects);
	glGenVertexArrays(1, &this->vertexArrayObject);
	glBindVertexArray(this->vertexArrayObject);

	this->textureIndices = new int[160000];

	if(houseLayer) {
		for(int x = 0; x < 400; x++) {
			for(int y = 0; y < 400; y++) {
				this->tiles[x][y] = 35;
			}
		}

		int offsetX = 2;
		int offsetY = 2;
		int maxX = 10;
		int maxY = 10;
		for(int x = offsetX; x < maxX + offsetX; x++) {
			for(int y = offsetY; y < maxY + offsetY; y++) {
				if(x == offsetX) {
					this->tiles[x][y]	= 30;
				}
				else if(y == offsetY) {
					this->tiles[x][y]	= 26;
				}
				else if(x == maxX + offsetX - 1) {
					this->tiles[x][y]	= 32;
				}
				else if(y == maxY + offsetY - 1) {
					this->tiles[x][y]	= 0;
				}
			}
		}

		this->tiles[offsetX][offsetY] = 23;
		this->tiles[offsetX][maxY - 1 + offsetY] = 28;
		this->tiles[maxX - 1 + offsetX][offsetY] = 19;
		this->tiles[maxX - 1 + offsetX][maxY - 1 + offsetY] = 24;
	}
	else {
		for(int x = 0; x < 400; x++) {
			for(int y = 0; y < 400; y++) {
				this->tiles[x][y] = 2;
			}
		}
	}

	int width = 400;
	int height = 400;
	int total = 0;

	int z = houseLayer ? 1 : 0;

	unsigned int size = 400;
	for(unsigned i = 0; i < size * size; i++) {
		glm::ivec2 coordinate = tilemath::indexToCoordinate(i, size);
		this->offsets[i][0] = coordinate.x * 0.06f / 2 + coordinate.y * 0.06f / 2;
		this->offsets[i][1] = -(coordinate.x * -0.06f / 4 + coordinate.y * 0.06f / 4 - z * 0.0365625);
		this->textureIndices[i] = this->tiles[coordinate.x][coordinate.y];
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

	// load texture indices
	{
		glBindBuffer(GL_ARRAY_BUFFER, this->vertexBufferObjects[3]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(int) * 160000, this->textureIndices, GL_STATIC_DRAW);

		glVertexAttribIPointer(3, 1, GL_INT, 0, 0);
		glVertexAttribDivisor(3, 1);
		glEnableVertexAttribArray(3);
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

	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 400 * 400);
}
