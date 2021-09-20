#include <glad/gl.h>
#include "chunk.h"

#include <random>

#include "../basic/camera.h"
#include "chunkContainer.h"
#include "../util/doubleDimension.h"
#include "../basic/line.h"
#include "overlappingTile.h"
#include "tileMath.h"

PNGImage* Chunk::Image = nullptr;
GLuint Chunk::Texture = GL_INVALID_INDEX;

glm::vec2 Chunk::Offsets[Chunk::Size * Chunk::Size * 15];
GLuint Chunk::VertexBufferObjects[3] = {GL_INVALID_INDEX, GL_INVALID_INDEX, GL_INVALID_INDEX};

void initOverlappingTileWrapper(Chunk* chunk, OverlappingTileWrapper** tile) {
	*tile = nullptr;
}

Chunk::Chunk() : InstancedRenderObjectContainer(false) {
	// initialize dynamic static data
	if(Image == nullptr) {
		glGenBuffers(3, Chunk::VertexBufferObjects);
		glGenTextures(1, &Chunk::Texture);

		// load image
		{	
			Chunk::Image = new PNGImage("data/spritesheet.png");
			glBindTexture(GL_TEXTURE_2D, Chunk::Texture);

			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				Chunk::Image->getFormat(),
				Chunk::Image->width,
				Chunk::Image->height,
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

		// pre-calculate offsets
		{
			for(unsigned i = 0; i < Size * Size; i++) {
				for(unsigned z = 0; z < 15; z++) {
					glm::ivec2 coordinate = tilemath::indexToCoordinate(i, Size);
					Chunk::Offsets[i + z * Size * Size] = tilemath::tileToScreen(glm::vec3(coordinate, z));
				}
			}

			glBindBuffer(GL_ARRAY_BUFFER, Chunk::VertexBufferObjects[0]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * Chunk::Size * Chunk::Size * 15, &Chunk::Offsets[0], GL_STATIC_DRAW);
		}

		// vertices for square
		{
			glBindBuffer(GL_ARRAY_BUFFER, Chunk::VertexBufferObjects[1]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(Chunk::Vertices), Chunk::Vertices, GL_STATIC_DRAW);
		}

		// uvs for square
		{
			glBindBuffer(GL_ARRAY_BUFFER, Chunk::VertexBufferObjects[2]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(Chunk::UVs), Chunk::UVs, GL_STATIC_DRAW);
		}
	}

	this->height = ((double)rand() / (RAND_MAX)) * 10 + 1;

	glGenBuffers(1, this->vertexBufferObjects);
	glGenVertexArrays(1, &this->vertexArrayObject);
	glBindVertexArray(this->vertexArrayObject);

	this->textureIndices = new int[Size * Size * this->height];
	for(unsigned i = 0; i < Size * Size * this->height; i++) {
		this->textureIndices[i] = 2;
	}
	
	// load vertices
	{
		glBindBuffer(GL_ARRAY_BUFFER, Chunk::VertexBufferObjects[1]);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);
	}

	// load uvs
	{
		glBindBuffer(GL_ARRAY_BUFFER, Chunk::VertexBufferObjects[2]);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);
	}

	// load offsets
	{
		glBindBuffer(GL_ARRAY_BUFFER, Chunk::VertexBufferObjects[0]);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribDivisor(2, 1);
		glEnableVertexAttribArray(2);
	}

	// load texture indices
	{
		glBindBuffer(GL_ARRAY_BUFFER, this->vertexBufferObjects[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(int) * Chunk::Size * Chunk::Size * this->height, this->textureIndices, GL_STATIC_DRAW);

		glVertexAttribIPointer(3, 1, GL_INT, 0, 0);
		glVertexAttribDivisor(3, 1);
		glEnableVertexAttribArray(3);
	}

	glBindVertexArray(0); // turn off vertex array object

	this->defineBounds();
}

void Chunk::setPosition(glm::uvec2 position) {
	this->position = position;
	this->screenSpacePosition = tilemath::tileToScreen(glm::vec3((unsigned int)Size * this->position, 0.0));
	this->defineBounds();
}

glm::uvec2& Chunk::getPosition() {
	return this->position;
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

void Chunk::renderChunk(double deltaTime, RenderContext &context) {
	// TODO make smart shader binding
	#ifdef EGGINE_DEBUG
	this->drawCalls = 0;
	#endif
	
	Camera* camera = context.camera;
	if(!(
		camera->left > this->right
		|| camera->right < this->left
		|| camera->top < this->bottom
		|| camera->bottom > this->top
	)) {
		unsigned int total = Chunk::Size * Chunk::Size * this->height;
		unsigned int lastOverlappingIndex = 0;
		size_t leftOff = 0;
		OverlappingTileWrapper* tile = nullptr;

		// TODO handle wall draw order for overlapping tiles
		// handle overlapping tiles
		for(size_t i = 0; i < this->overlappingTiles.array.head && (tile = this->overlappingTiles.array[i])->index < total; i++) { // go through overlapping tiles
			if(lastOverlappingIndex - 1 != tile->index) {
				// draw [last, lastOverlappingIndex - tile.index + last)
				// we need to reset the pipeline since we could have drawn an overlapping tile before this batch
				glBindVertexArray(this->vertexArrayObject);
				glUniform2f(ChunkContainer::Uniforms[2], this->screenSpacePosition.x, this->screenSpacePosition.y);
				glDrawArraysInstancedBaseInstance(GL_TRIANGLE_STRIP, 0, 4, tile->index - lastOverlappingIndex + 1, lastOverlappingIndex);
				#ifdef EGGINE_DEBUG
				this->drawCalls++;
				#endif
			}

			tile->tile->render(deltaTime, context);

			lastOverlappingIndex = tile->index + 1;

			leftOff = i + 1;
		}

		glBindVertexArray(this->vertexArrayObject);
		glUniform2f(ChunkContainer::Uniforms[2], this->screenSpacePosition.x, this->screenSpacePosition.y);
		glDrawArraysInstancedBaseInstance(GL_TRIANGLE_STRIP, 0, 4, total - lastOverlappingIndex, lastOverlappingIndex);
		#ifdef EGGINE_DEBUG
		this->drawCalls++;
		#endif

		// draw overlapping tiles above the height of the chunk
		for(size_t i = leftOff; i < this->overlappingTiles.array.head; i++) {
			this->overlappingTiles.array[i]->tile->render(deltaTime, context);
		}

		#ifdef EGGINE_DEBUG
		this->isCulled = false;
		#endif
	}
	#ifdef EGGINE_DEBUG
	else {
		this->isCulled = true;
	}
	#endif
}

void Chunk::addOverlappingTile(OverlappingTile* tile) {
	glm::uvec2 relativePosition = glm::uvec2(tile->getPosition()) - this->position * (unsigned int)Chunk::Size;
	long index = tilemath::coordinateToIndex(relativePosition, Chunk::Size) + Chunk::Size * Chunk::Size * tile->getPosition().z;

	OverlappingTileWrapper* wrapper = new OverlappingTileWrapper();
	wrapper->index = index;
	wrapper->tile = tile;

	this->overlappingTiles.insert(wrapper);
	tile->setChunk(this);
}
	
void Chunk::removeOverlappingTile(OverlappingTile* tile) {
	
}
