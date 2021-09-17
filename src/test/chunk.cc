#include <glad/gl.h>
#include "chunk.h"

#include <random>

#include "../basic/camera.h"
#include "../util/doubleDimension.h"
#include "../basic/line.h"
#include "tileMath.h"

PNGImage* Chunk::Image = nullptr;
GLuint Chunk::Texture = 0;

Chunk::Chunk(glm::vec2 position) : InstancedRenderObjectContainer(false) {
	this->position = position;
	this->screenSpacePosition = tilemath::tileToScreen(glm::vec3(Size * this->position, 0.0));

	this->height = ((double)rand() / (RAND_MAX)) * 10 + 1;

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
			this->offsets[i + z * Size * Size] = tilemath::tileToScreen(glm::vec3(coordinate, z)) + this->screenSpacePosition;
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
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * Chunk::Size * Chunk::Size * this->height, &this->offsets[0], GL_STATIC_DRAW);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribDivisor(2, 1);
		glEnableVertexAttribArray(2);
	}

	// load texture indices
	{
		glBindBuffer(GL_ARRAY_BUFFER, this->vertexBufferObjects[3]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(int) * Chunk::Size * Chunk::Size * this->height, this->textureIndices, GL_STATIC_DRAW);

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

	this->defineBounds();
}

void Chunk::defineBounds() {
	glm::vec2 bias(-0.5, -(32.0 / 2.0 + 39.0 * 2.0 + 2) / 128.0);

	glm::vec2 top = tilemath::tileToScreen(glm::vec3(Size, 0, this->height)) + this->screenSpacePosition + bias;
	glm::vec2 right = tilemath::tileToScreen(glm::vec3(Size, Size, 0)) + this->screenSpacePosition + bias;
	glm::vec2 bottom = tilemath::tileToScreen(glm::vec3(0, Size, 0)) + this->screenSpacePosition + bias;
	glm::vec2 left = tilemath::tileToScreen(glm::vec3(0, 0, 0)) + this->screenSpacePosition + bias;

	this->left = left.x;
	this->right = right.x;
	this->top = top.y;
	this->bottom = bottom.y;
}

void Chunk::buildDebugLines() {
	this->debugLine = new Line();
	this->debugLine->addPosition(glm::vec2(this->left, this->bottom));
	this->debugLine->addPosition(glm::vec2(this->right, this->bottom));
	this->debugLine->addPosition(glm::vec2(this->right, this->top));
	this->debugLine->addPosition(glm::vec2(this->left, this->top));
	this->debugLine->addPosition(glm::vec2(this->left, this->bottom));
	this->debugLine->commit();
}

void Chunk::render(double deltaTime, RenderContext &context) {
	Camera* camera = context.camera;
	if(!(
		camera->left > this->right
		|| camera->right < this->left
		|| camera->top < this->bottom
		|| camera->bottom > this->top
	)) {
		glBindVertexArray(this->vertexArrayObject);
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, Chunk::Size * Chunk::Size * this->height);
	}
}
