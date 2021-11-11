#include "chunk.h"

#include <random>

#include "../basic/camera.h"
#include "chunkContainer.h"
#include "../util/doubleDimension.h"
#include "../engine/engine.h"
#include "interweavedTile.h"
#include "layer.h"
#include "../basic/line.h"
#include "overlappingTile.h"
#include "../resources/resourceManager.h"
#include "../resources/spriteSheet.h"
#include "tileMath.h"

glm::vec2 Chunk::OffsetsSource[Chunk::Size * Chunk::Size * Chunk::MaxHeight];
render::VertexBuffer* Chunk::Offsets = nullptr;
tsl::robin_map< pair<tilemath::Rotation, tilemath::Rotation>, vector<long>> Chunk::Rotations = tsl::robin_map< pair<tilemath::Rotation, tilemath::Rotation>, vector<long>>();

void initInterweavedTileWrapper(Chunk* chunk, InterweavedTileWrapper* tile) {
	*tile = {};
}

int compareInterweavedTile(InterweavedTileWrapper* a, InterweavedTileWrapper* b) {
	InterweavedTileWrapper* a2 = (InterweavedTileWrapper*)a;
	InterweavedTileWrapper* b2 = (InterweavedTileWrapper*)b;

	if(*a2 > *b2) {
		return 1;
	}
	else if(*a2 < *b2) {
		return -1;
	}
	else {
		return 0;
	}
}

Chunk::Chunk(ChunkContainer* container) : InstancedRenderObjectContainer(false) {
	this->container = container;
	
	if(Chunk::Offsets == nullptr) {
		Chunk::BuildOffsets(container->getRotation());
	}

	if(Chunk::Rotations.size() == 0) {
		for(int a = 0; a < 4; a++) {
			for(int b = 0; b < 4; b++) {
				if(a != b) {
					auto rotationPair = pair((tilemath::Rotation)a, (tilemath::Rotation)b);
					Chunk::Rotations[rotationPair].resize(Chunk::Size * Chunk::Size);
					for(unsigned int oldIndex = 0; oldIndex < Chunk::Size * Chunk::Size; oldIndex++) {
						glm::ivec2 intermediatePosition = tilemath::indexToCoordinate(oldIndex, Chunk::Size, (tilemath::Rotation)a);
						long newIndex = tilemath::coordinateToIndex(intermediatePosition, Chunk::Size, (tilemath::Rotation)b);
						Chunk::Rotations[rotationPair][oldIndex] = newIndex;
					}
				}
			}
		}
	}

	// this->height = ((double)rand() / (RAND_MAX)) * 10 + 1;
	this->height = 5;

	this->vertexBuffer = new render::VertexBuffer(&engine->renderWindow);
	this->vertexAttributes = new render::VertexAttributes(&engine->renderWindow);

	this->textureIndices = new int[Size * Size * Chunk::MaxHeight];
	for(unsigned i = 0; i < Size * Size * Chunk::MaxHeight; i++) {
		this->textureIndices[i] = 0;
	}

	// load vertices
	{
		this->vertexAttributes->addVertexAttribute(ChunkContainer::Vertices, 0, 2, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec2), 0);
	}

	// load uvs
	{
		this->vertexAttributes->addVertexAttribute(ChunkContainer::UVs, 1, 2, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec2), 0);
	}

	// load offsets
	{
		this->vertexAttributes->addVertexAttribute(Chunk::Offsets, 2, 2, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec2), 1);
	}

	// load texture indices
	{
		this->vertexBuffer->setData(this->textureIndices, sizeof(int) * Chunk::Size * Chunk::Size * Chunk::MaxHeight, alignof(int));
		this->vertexAttributes->addVertexAttribute(this->vertexBuffer, 3, 1, render::VERTEX_ATTRIB_INT, 0, sizeof(int), 1);
	}

	// load colors
	{
		this->vertexAttributes->addVertexAttribute(ChunkContainer::Colors, 4, 4, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec4), 0);
	}

	this->defineBounds();
}

Chunk::~Chunk() {
	delete this->textureIndices;
	if(this->debugLine != nullptr) {
		delete this->debugLine;
	}
}

void Chunk::BuildOffsets(tilemath::Rotation rotation) {
	// pre-calculate offsets
	for(unsigned i = 0; i < Size * Size; i++) {
		for(unsigned z = 0; z < Chunk::MaxHeight; z++) {
			glm::ivec2 coordinate = tilemath::indexToCoordinate(i, Size, rotation);
			Chunk::OffsetsSource[i + z * Size * Size] = tilemath::tileToScreen(glm::vec3(coordinate, z), rotation);
		}
	}

	if(Chunk::Offsets == nullptr) {
		Chunk::Offsets = new render::VertexBuffer(&engine->renderWindow);
	}
	Chunk::Offsets->setData(&Chunk::OffsetsSource[0], sizeof(glm::vec2) * Chunk::Size * Chunk::Size * Chunk::MaxHeight, alignof(glm::vec2));
}

void Chunk::setPosition(glm::uvec2 position) {
	this->position = position;
	this->screenSpacePosition = tilemath::tileToScreen(glm::vec3((unsigned int)Size * this->position, 0.0), this->container->getRotation());
	this->defineBounds();
}

glm::uvec2& Chunk::getPosition() {
	return this->position;
}

void Chunk::defineBounds() {
	tilemath::Rotation rotation = this->container->getRotation();
	int topHeight = 0, rightHeight = 0, bottomHeight = 0, leftHeight = 0;
	glm::vec2 bias;
	switch(rotation) {
		case tilemath::ROTATION_0_DEG: {
			bias = glm::vec2(-0.5, -(32.0 / 2.0 + 39.0 * 2.0 + 2) / 128.0);
			topHeight = this->height;
			break;
		}

		case tilemath::ROTATION_90_DEG: {
			bias = glm::vec2(0, -(32.0 + 39.0 * 2.0 + 16) / 128.0);
			rightHeight = this->height;
			break;
		}

		case tilemath::ROTATION_180_DEG: {
			bias = glm::vec2(0.5, -(32.0 / 2.0 + 39.0 * 2.0 + 2) / 128.0);
			bottomHeight = this->height;
			break;
		}

		case tilemath::ROTATION_270_DEG: {
			bias = glm::vec2(0, -(39.0 * 2.0 + 2 - 16) / 128.0);
			leftHeight = this->height;
			break;
		}
	}

	glm::vec2 top = tilemath::tileToScreen(glm::vec3(Size, 0, topHeight), rotation) + this->screenSpacePosition + bias;
	glm::vec2 right = tilemath::tileToScreen(glm::vec3(Size, Size, rightHeight), rotation) + this->screenSpacePosition + bias;
	glm::vec2 bottom = tilemath::tileToScreen(glm::vec3(0, Size, bottomHeight), rotation) + this->screenSpacePosition + bias;
	glm::vec2 left = tilemath::tileToScreen(glm::vec3(0, 0, leftHeight), rotation) + this->screenSpacePosition + bias;

	switch(rotation) {
		case tilemath::ROTATION_0_DEG: {
			this->left = left.x;
			this->right = right.x;
			this->top = top.y;
			this->bottom = bottom.y;
			break;
		}

		case tilemath::ROTATION_90_DEG: {
			this->left = top.x;
			this->right = bottom.x;
			this->top = right.y;
			this->bottom = left.y;
			break;
		}

		case tilemath::ROTATION_180_DEG: {
			this->left = right.x;
			this->right = left.x;
			this->top = bottom.y;
			this->bottom = top.y;
			break;
		}
		
		case tilemath::ROTATION_270_DEG: {
			this->left = bottom.x;
			this->right = top.x;
			this->top = left.y;
			this->bottom = right.y;
			break;
		}
	}
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

void Chunk::updateRotation(tilemath::Rotation rotation) {
	int* newTextureIndices = (int*)calloc(Chunk::Size * Chunk::Size * Chunk::MaxHeight, sizeof(int)); // this is slightly faster than new
	for(unsigned int i = 0;  i < Chunk::Size * Chunk::Size * Chunk::MaxHeight; i++) {
		unsigned int height = i / (Chunk::Size * Chunk::Size);
		long newIndex = Chunk::Rotations[pair(this->oldRotation, rotation)][i % (Chunk::Size * Chunk::Size)];

		resources::SpriteFacingInfo* facingsMap;
		if((facingsMap = ChunkContainer::Image->getSpriteInfo(this->textureIndices[i]).facingsMap) != nullptr) {
			newTextureIndices[newIndex + height * Chunk::Size * Chunk::Size] = facingsMap->rotateFacing(ChunkContainer::Image->getSpriteInfo(this->textureIndices[i]).facing, this->oldRotation, rotation);
		}
		else {
			newTextureIndices[newIndex + height * Chunk::Size * Chunk::Size] = this->textureIndices[i];
		}
	}

	delete[] this->textureIndices;
	this->textureIndices = newTextureIndices;
	this->vertexBuffer->setData(this->textureIndices, sizeof(int) * Chunk::Size * Chunk::Size * Chunk::MaxHeight, alignof(int));

	this->defineBounds();

	for(unsigned int i = 0; i <= this->maxLayer; i++) {
		if(this->layers[i] != nullptr) {
			this->layers[i]->updateRotation(this->oldRotation, rotation);
		}
	}

	for(unsigned int i = 0; i < this->interweavedTiles.array.head; i++) {
		InterweavedTile* tile = this->interweavedTiles.array[i].tile;
		glm::uvec2 relativePosition = glm::uvec2(tile->getPosition()) - this->position * (unsigned int)Chunk::Size;
		this->interweavedTiles.array[i].index = tilemath::coordinateToIndex(relativePosition, Chunk::Size, rotation) + Chunk::Size * Chunk::Size * tile->getPosition().z;
		tile->updateRotation(this->oldRotation, rotation);
	}
	this->interweavedTiles.sort();

	this->oldRotation = rotation;
}

size_t Chunk::renderWithInterweavedTiles(size_t startInterweavedIndex, size_t startIndex, size_t amount, double deltaTime, RenderContext &context) {
	unsigned int total = Chunk::Size * Chunk::Size * this->height;
	unsigned int lastIndex = startIndex;
	size_t leftOff = startInterweavedIndex;
	size_t limit = startIndex + amount;
	InterweavedTileWrapper* tile = nullptr;

	for(
		size_t i = startInterweavedIndex;
		i < this->interweavedTiles.array.head && (tile = &this->interweavedTiles.array[i])->index < limit;
		i++
	) { // go through interweaved tiles
		int overlapBias = ChunkContainer::Image->drawOntopOfOverlap(this->textureIndices[tile->index]) ? 0 : 1;
		if(lastIndex - 1 != tile->index) {
			// draw [last, lastInterweavedIndex - tile.index + last)
			// we need to reset the pipeline since we could have drawn an overlapping tile before this batch
			this->vertexAttributes->bind();
			engine->renderWindow.draw(render::PRIMITIVE_TRIANGLE_STRIP, 0, 4, lastIndex, tile->index - lastIndex + overlapBias);
			#ifdef EGGINE_DEBUG
			this->drawCalls++;
			#endif
		}

		tile->tile->render(deltaTime, context);

		lastIndex = tile->index + overlapBias;

		leftOff = i + 1;
	}

	this->vertexAttributes->bind();
	engine->renderWindow.draw(render::PRIMITIVE_TRIANGLE_STRIP, 0, 4, lastIndex, limit - lastIndex);
	#ifdef EGGINE_DEBUG
	this->drawCalls++;
	#endif

	return leftOff;
}

void Chunk::renderChunk(double deltaTime, RenderContext &context) {
	#ifdef EGGINE_DEBUG
	this->drawCalls = 0;
	#endif

	int interweavedIndex = 0;

	struct VertexBlock {
		glm::mat4 projection;
		glm::vec2 chunkScreenSpace;
		float spritesheetWidth;
		float spritesheetHeight;
		float spriteWidth;
		float spriteHeight;
		float spritesOnRow;
	} vb;
	vb.projection = context.camera->projectionMatrix;
	vb.chunkScreenSpace = this->screenSpacePosition;
	vb.spritesheetWidth = 1057;
	vb.spritesheetHeight = 391;
	vb.spriteWidth = 64.0f;
	vb.spriteHeight = 128.0f;
	vb.spritesOnRow = floor(vb.spritesheetWidth / vb.spriteWidth);

	struct FragmentBlock {
		glm::vec4 color;
	} fb;
	fb.color = glm::vec4(1, 1, 1, 1);

	ChunkContainer::Program->bindUniform("vertexBlock", &vb, sizeof(vb));
	
	Camera* camera = context.camera;
	if(!(
		camera->left > this->right
		|| camera->right < this->left
		|| camera->top < this->bottom
		|| camera->bottom > this->top
	)) {
		// try to draw as many layers at once as we can
		unsigned int start = 0;
		unsigned int end = 0;
		size_t interweavedIndex = 0;
		bool rendered = false;
		for(unsigned int i = 0; i < this->height; i++) {
			end = i;
			rendered = false;

			if(this->getLayer(i) != nullptr) { // check if we need to stop and render the layer
				this->vertexAttributes->bind();
				// engine->renderWindow.draw(render::PRIMITIVE_TRIANGLE_STRIP, 0, 4, start * Chunk::Size * Chunk::Size, (end - start + 1) * Chunk::Size * Chunk::Size);
				interweavedIndex = this->renderWithInterweavedTiles(interweavedIndex, start * Chunk::Size * Chunk::Size, (end - start + 1) * Chunk::Size * Chunk::Size, deltaTime, context);

				this->getLayer(i)->render(deltaTime, context);

				start = i + 1;
				end = i;
				rendered = true;
			}
		}

		if(!rendered) { // render remaining tiles at top of the chunk
			this->vertexAttributes->bind();
			// engine->renderWindow.draw(render::PRIMITIVE_TRIANGLE_STRIP, 0, 4, start * Chunk::Size * Chunk::Size, (end - start + 1) * Chunk::Size * Chunk::Size);
			interweavedIndex = this->renderWithInterweavedTiles(interweavedIndex, start * Chunk::Size * Chunk::Size, (end - start + 1) * Chunk::Size * Chunk::Size, deltaTime, context);
		}

		// render remaining overlapping tiles
		for(unsigned int i = end; i <= this->maxLayer; i++) {
			if(this->getLayer(i) != nullptr) {
				this->getLayer(i)->render(deltaTime, context);
			}
		}

		// draw overlapping tiles above the height of the chunk
		for(size_t i = interweavedIndex; i < this->interweavedTiles.array.head; i++) {
			this->interweavedTiles.array[i].tile->render(deltaTime, context);
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
	Layer* found = this->getLayer(tile->getPosition().z);
	if(found == nullptr) {
		found = this->layers[tile->getPosition().z] = new Layer(this);
		this->maxLayer = max(tile->getPosition().z, this->maxLayer);
	}

	this->overlappingTiles.insert(tile);

	found->add(tile);
}

void Chunk::updateOverlappingTile(OverlappingTile* tile) {
	Layer* found = this->getLayer(tile->getPosition().z);
	if(found == nullptr) {
		found = this->layers[tile->getPosition().z] = new Layer(this);
		this->maxLayer = max(tile->getPosition().z, this->maxLayer);
	}
	
	if(found != tile->getLayer()) {
		tile->getLayer()->remove(tile);
		found->add(tile);
	}
	else {
		found->update(tile);
	}
}
	
void Chunk::removeOverlappingTile(OverlappingTile* tile) {
	if(tile->getLayer() != nullptr) {
		tile->getLayer()->remove(tile);
	}

	this->overlappingTiles.erase(tile);
}

void Chunk::addInterweavedTile(InterweavedTile* tile) {
	glm::uvec2 relativePosition = glm::uvec2(tile->getPosition()) - this->position * (unsigned int)Chunk::Size;
	unsigned int index = tilemath::coordinateToIndex(relativePosition, Chunk::Size, this->container->getRotation()) + Chunk::Size * Chunk::Size * tile->getPosition().z;

	this->interweavedTiles.insert(InterweavedTileWrapper {
		index: index,
		tile: tile,
	});
}

void Chunk::updateInterweavedTile(InterweavedTile* tile) {
	// find the tile and update its index
	glm::uvec2 relativePosition = glm::uvec2(tile->getPosition()) - this->position * (unsigned int)Chunk::Size;
	unsigned int index = tilemath::coordinateToIndex(relativePosition, Chunk::Size, this->container->getRotation()) + Chunk::Size * Chunk::Size * tile->getPosition().z;

	for(size_t i = 0; i < this->interweavedTiles.array.head; i++) {
		if(this->interweavedTiles.array[i].tile == tile) {
			this->interweavedTiles.array[i].index = index;
		}
	}

	this->interweavedTiles.sort();
}

void Chunk::removeInterweavedTile(InterweavedTile* tile) {
	if(this->interweavedTiles.array.head != 0) { // signifies that the array has been deallocated
		this->interweavedTiles.remove(InterweavedTileWrapper {
			index: 0,
			tile: tile,
		});
	}	
}

Layer* Chunk::getLayer(unsigned int z) {
	auto found = this->layers.find(z);
	if(found == this->layers.end()) {
		return nullptr;
	}
	return found.value();
}

void Chunk::setTileTexture(glm::uvec3 position, unsigned int spritesheetIndex) {
	glm::uvec2 relativePosition = glm::uvec2(position.x, position.y) - this->position * (unsigned int)Chunk::Size;
	unsigned int index = tilemath::coordinateToIndex(relativePosition, Chunk::Size, this->container->getRotation()) + position.z * Chunk::Size * Chunk::Size;

	this->vertexBuffer->setSubData(&spritesheetIndex, 1, index * sizeof(unsigned int));
	this->textureIndices[index] = spritesheetIndex;

	this->height = max(this->height, position.z + 1);
}

int Chunk::getTileTexture(glm::uvec3 position) {
	glm::uvec2 relativePosition = glm::uvec2(position.x, position.y) - this->position * (unsigned int)Chunk::Size;
	unsigned int index = tilemath::coordinateToIndex(relativePosition, Chunk::Size, this->container->getRotation()) + position.z * Chunk::Size * Chunk::Size;

	if(index > Chunk::Size * Chunk::Size * Chunk::MaxHeight) {
		return 0;
	}
	return this->textureIndices[index];
}

void Chunk::setTileTextureByIndex(size_t index, unsigned int spritesheetIndex) {
	if(index < Chunk::Size * Chunk::Size * Chunk::MaxHeight) {
		unsigned int height = index / (Chunk::Size * Chunk::Size);
		this->height = max(this->height, height + 1);
		this->vertexBuffer->setSubData(&spritesheetIndex, 1, index * sizeof(unsigned int)); // TODO improve efficiency
		this->textureIndices[index] = spritesheetIndex;
	}
}

int Chunk::getTileTextureByIndex(size_t index) {
	if(index < Chunk::Size * Chunk::Size * Chunk::MaxHeight) {
		return this->textureIndices[index];
	}
	return -1;
}

bool InterweavedTileWrapper::operator<(const InterweavedTileWrapper &other) {
	if(this->index == other.index) {
		return this->tile->getZIndex() < other.tile->getZIndex();
	}
	return this->index < other.index;
}

bool InterweavedTileWrapper::operator>(const InterweavedTileWrapper &other) {
	if(this->index == other.index) {
		return this->tile->getZIndex() > other.tile->getZIndex();
	}
	return this->index > other.index;
}

bool InterweavedTileWrapper::operator==(const InterweavedTileWrapper &other) {
	return this->tile == other.tile;
}
