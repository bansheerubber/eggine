#include "chunkContainer.h"

#define FMT_HEADER_ONLY
#include <fmt/format.h>

#include "../basic/camera.h"
#include "chunk.h"
#include "../engine/debug.h"
#include "../engine/engine.h"
#include "overlappingTile.h"
#include "../renderer/shader.h"
#include "tileMath.h"

render::Program* ChunkContainer::Program = nullptr;
resources::SpriteSheet* ChunkContainer::Image = nullptr;

void initChunk(class ChunkContainer* container, class Chunk* chunk) {
	new((void*)chunk) Chunk();
}

ChunkContainer::ChunkContainer() {
	engine->registerBind("chunk.selectTile", this);
	engine->registerBind("chunk.selectTileUp", this);
	engine->registerBind("chunk.selectTileDown", this);
	engine->registerBind("chunk.selectTileLeft", this);
	engine->registerBind("chunk.selectTileRight", this);

	engine->registerBindAxis("chunk.xAxis", this);
	engine->registerBindAxis("chunk.yAxis", this);
	
	if(ChunkContainer::Program == nullptr) {
		render::Shader* vertexShader = new render::Shader(&engine->renderWindow);
		vertexShader->load(getShaderSource("shaders/tile.vert"), render::SHADER_VERTEX);

		render::Shader* fragmentShader = new render::Shader(&engine->renderWindow);
		fragmentShader->load(getShaderSource("shaders/tile.frag"), render::SHADER_FRAGMENT);

		ChunkContainer::Program = new render::Program(&engine->renderWindow);
		ChunkContainer::Program->addShader(vertexShader);
		ChunkContainer::Program->addShader(fragmentShader);
	}

	if(ChunkContainer::Image == nullptr) {
		ChunkContainer::Image = (resources::SpriteSheet*)engine->manager->metadataToResources(
			engine->manager->carton->database.get()->equals("extension", ".png")->exec()
		)[0];
	}
}

ChunkContainer::~ChunkContainer() {
	for(size_t i = 0; i < this->renderOrder.head; i++) {
		this->renderOrder[i].~Chunk(); // because of how dynamic array reallocs we have to do this bullshit
	}
}

void ChunkContainer::addChunk(glm::uvec2 position) {
	this->renderOrder[this->renderOrder.head].setPosition(position);
	this->renderOrder.pushed();

	this->size = ceil(sqrt(this->renderOrder.head));
}

Chunk& ChunkContainer::getChunk(size_t index) {
	if(index >= this->size * this->size) {
		printf("ChunkContainer::getChunk(): chunk index out of bounds\n");
		exit(1);
	}

	return this->renderOrder[index];
}

size_t ChunkContainer::getChunkCount() {
	return this->renderOrder.head;
}

void ChunkContainer::render(double deltaTime, RenderContext &context) {
	ChunkContainer::Program->bind();

	ChunkContainer::Program->bindTexture("spriteTexture", 0);
	ChunkContainer::Image->texture->bind(0);

	#ifdef EGGINE_DEBUG
	size_t chunksRendered = 0;
	size_t tilesRendered = 0;
	size_t tiles = 0;
	size_t drawCalls = 0;
	size_t overlappingCalls = 0;
	#endif

	for(size_t i = 0; i < this->renderOrder.head; i++) {
		Chunk &chunk = this->renderOrder[i];
		chunk.renderChunk(deltaTime, context);

		#ifdef EGGINE_DEBUG
		if(!chunk.isCulled) {
			chunksRendered++;
			tilesRendered += Chunk::Size * Chunk::Size * chunk.height;
			drawCalls += chunk.drawCalls;
			overlappingCalls += chunk.overlappingTiles.array.head;
		}
		
		tiles += Chunk::Size * Chunk::Size * chunk.height;
		#endif
	}

	#ifdef EGGINE_DEBUG
	engine->debug.addInfoMessage(fmt::format("{}/{} chunks rendered", chunksRendered, this->renderOrder.head));
	engine->debug.addInfoMessage(fmt::format("{}/{} tiles rendered", tilesRendered, tiles));
	engine->debug.addInfoMessage(fmt::format("{} chunk draw calls, {} overlapping calls", drawCalls, overlappingCalls));
	#endif
}

void ChunkContainer::addOverlappingTile(OverlappingTile* tile) {
	this->setOverlappingTileChunk(tile);
}

void ChunkContainer::setOverlappingTileChunk(OverlappingTile* tile) {
	glm::uvec2 chunkPosition = tile->getPosition() / (unsigned int)Chunk::Size;

	if(chunkPosition.x >= this->size || chunkPosition.y >= this->size) {
		printf("ChunkContainer::setOverlappingTileChunk(): chunk position out of bounds\n");
		exit(1);
	}

	long index = tilemath::coordinateToIndex(chunkPosition, this->size);
	this->renderOrder[index].addOverlappingTile(tile);
}

void ChunkContainer::onBind(string &bind, binds::Action action) {
	if(bind == "chunk.selectTile" && action == binds::PRESS) {
		glm::vec2 world = engine->camera->mouseToWorld(engine->mouse);

		// glm::mat2 basis = glm::mat2(
		// 	cos(atan(1/2)), sin(atan(1/2)),
		// 	cos(atan(1/2)), -sin(atan(1/2))
		// );

		// inverse the basis and normalize the axes to get transformation matrix
		double cosine45deg = cos(M_PI / 4.0f);
		glm::mat2 inverseBasis = glm::mat2(
			cosine45deg, cosine45deg,
			-cosine45deg * 2.0f, cosine45deg * 2.0f
		);
		glm::ivec2 coordinates = (inverseBasis * world) * (float)cosine45deg * 2.0f;
		
		if(coordinates.x >= 0 && coordinates.y >= 0 && coordinates.x < this->size * Chunk::Size && coordinates.y < this->size * Chunk::Size) {
			this->tileSelectionSprite->setPosition(glm::uvec3(coordinates, 0));
		}
	}
	else if(
		(
			bind == "chunk.selectTileUp"
			|| bind == "chunk.selectTileDown"
			|| bind == "chunk.selectTileLeft"
			|| bind == "chunk.selectTileRight"
		) && action == binds::PRESS
	) {
		glm::ivec3 position(this->tileSelectionSprite->getPosition());
		if(bind == "chunk.selectTileUp") {
			position += glm::ivec3(1, -1, 0);
		}
		else if(bind == "chunk.selectTileDown") {
			position += glm::ivec3(-1, 1, 0);
		}
		else if(bind == "chunk.selectTileLeft") {
			position += glm::ivec3(-1, -1, 0);
		}
		else if(bind == "chunk.selectTileRight") {
			position += glm::ivec3(1, 1, 0);
		}

		if(position.x >= 0 && position.y >= 0 && position.x < this->size * Chunk::Size && position.y < this->size * Chunk::Size) {
			this->tileSelectionSprite->setPosition(position);
		}
	}
}

void ChunkContainer::onAxis(string &bind, double value) {
	if(bind == "chunk.xAxis" || bind == "chunk.yAxis") {
		double cosine45deg = cos(M_PI / 4.0f);
		glm::mat2 inverseBasis = glm::mat2(
			cosine45deg, cosine45deg,
			-cosine45deg * 2.0f, cosine45deg * 2.0f
		);
		glm::vec2 position(engine->camera->getPosition());
		position.x += 0.5;
		position.y = -position.y;
		glm::ivec2 coordinates = (inverseBasis * position) * (float)cosine45deg * 2.0f;
		
		if(coordinates.x >= 0 && coordinates.y >= 0 && coordinates.x < this->size * Chunk::Size && coordinates.y < this->size * Chunk::Size) {
			this->tileSelectionSprite->setPosition(glm::uvec3(coordinates, 0));
		}
	}
}

void ChunkContainer::commit() {
	if(this->tileSelectionSprite == nullptr) {
		this->tileSelectionSprite = new OverlappingTile(this);
		this->tileSelectionSprite->setTexture(17);
		this->tileSelectionSprite->setPosition(glm::uvec3(0, 0, 0));
	}
}
