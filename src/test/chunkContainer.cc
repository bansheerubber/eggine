#include "chunkContainer.h"

#define FMT_HEADER_ONLY
#include <fmt/format.h>

#include "../basic/camera.h"
#include "character.h"
#include "chunk.h"
#include "../engine/debug.h"
#include "../engine/engine.h"
#include "overlappingTile.h"
#include "../renderer/shader.h"
#include "team.h"
#include "tileMath.h"

render::Program* ChunkContainer::Program = nullptr;
resources::SpriteSheet* ChunkContainer::Image = nullptr;
render::VertexBuffer* ChunkContainer::Vertices = nullptr;
render::VertexBuffer* ChunkContainer::UVs = nullptr;
render::VertexBuffer* ChunkContainer::Colors = nullptr;

void initChunk(class ChunkContainer* container, class Chunk* chunk) {
	new((void*)chunk) Chunk(container);
}

ChunkContainer::ChunkContainer() {
	engine->registerBind("chunk.selectTile", this);
	engine->registerBind("chunk.mouseSelectTile", this);
	engine->registerBind("chunk.mouseRightClickTile", this);

	engine->registerBindAxis("chunk.xAxis", this);
	engine->registerBindAxis("chunk.yAxis", this);
	engine->registerBindAxis("chunk.mouseXAxis", this);
	engine->registerBindAxis("chunk.mouseYAxis", this);
	
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

	if(ChunkContainer::Vertices == nullptr) { // vertices for square
		ChunkContainer::Vertices = new render::VertexBuffer(&engine->renderWindow);
		ChunkContainer::Vertices->setData((glm::vec2*)&ChunkContainer::VerticesSource[0], sizeof(ChunkContainer::VerticesSource), alignof(glm::vec2));
	}

	
	if(ChunkContainer::UVs == nullptr) { // uvs for square
		ChunkContainer::UVs = new render::VertexBuffer(&engine->renderWindow);
		ChunkContainer::UVs->setData((glm::vec2*)&ChunkContainer::UVsSource[0], sizeof(ChunkContainer::UVsSource), alignof(glm::vec2));
	}

	if(ChunkContainer::Colors == nullptr) { // uvs for square
		ChunkContainer::Colors = new render::VertexBuffer(&engine->renderWindow);
		ChunkContainer::Colors->setData((glm::vec4*)&ChunkContainer::ColorsSource[0], sizeof(ChunkContainer::ColorsSource), alignof(glm::vec4));
	}

	this->renderOrder.allocate(8);

	// create torquescript object
	this->reference = esInstantiateObject(engine->eggscript, "ChunkContainer", this);

	this->playerTeam = new Team();
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
			overlappingCalls += chunk.layers.size();
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

bool ChunkContainer::isValidTilePosition(glm::ivec3 position) {
	if(
		position.x >= this->size * Chunk::Size 
		|| position.x < 0
		|| position.y >= this->size * Chunk::Size
		|| position.y < 0
		|| position.z < 0
	) {
		return false;
	}
	return true;
}

void ChunkContainer::selectCharacter(Character* character) {
	this->selectedCharacter = character;

	glm::uvec3 position = character->getPosition();
	if(position.z != 0) {
		position.z -= 1;
	}

	this->characterSelectionSprite->setPosition(position);
}

// called by the selected character when the characters position/etc changes
void ChunkContainer::updateCharacterPosition(Character* character, glm::uvec3 newPosition) {
	this->positionToCharacter.erase(character->getPosition());
	this->positionToCharacter[newPosition] = character;
	
	if(this->selectedCharacter == character) {
		if(newPosition.z != 0) {
			newPosition.z -= 1;
		}

		this->characterSelectionSprite->setPosition(newPosition);
	}
}

void ChunkContainer::setTile(glm::ivec3 position, int texture) {
	if(!this->isValidTilePosition(position)) {
		return;
	}

	glm::uvec2 chunkPosition = glm::uvec3(position) / (unsigned int)Chunk::Size;
	long index = tilemath::coordinateToIndex(chunkPosition, this->size);
	this->renderOrder[index].setTileTexture(position, texture);
}

int ChunkContainer::getTile(glm::ivec3 position) {
	if(!this->isValidTilePosition(position)) {
		return 0;
	}

	glm::uvec2 chunkPosition = glm::uvec3(position) / (unsigned int)Chunk::Size;
	long index = tilemath::coordinateToIndex(chunkPosition, this->size);
	return this->renderOrder[index].getTileTexture(position);
}

void ChunkContainer::selectTile(glm::ivec3 position, bool browsing) {
	if(!this->isValidTilePosition(position)) {
		return;
	}
	
	this->tileSelectionSprite->setPosition(position);

	esEntry arguments[2];
	esCreateVectorAt(&arguments[0], 3, (double)position.x, (double)position.y, (double)position.z);
	esCreateNumberAt(&arguments[1], browsing);
	esCallFunction(engine->eggscript, "onSelectTile", 2, arguments);
}

void ChunkContainer::rightClickTile(glm::ivec3 position) {
	esEntry arguments[1];
	esCreateVectorAt(&arguments[0], 3, (double)position.x, (double)position.y, (double)position.z);
	esCallFunction(engine->eggscript, "onRightClickTile", 1, arguments);
}

void ChunkContainer::hoverTile(glm::ivec3 position) {
	esEntry arguments[1];
	esCreateVectorAt(&arguments[0], 3, (double)position.x, (double)position.y, (double)position.z);
	esCallFunction(engine->eggscript, "onHoverTile", 1, arguments);
}

glm::ivec3 ChunkContainer::findCandidateSelectedTile(glm::vec2 world) {
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
	glm::vec3 _((inverseBasis * world) * (float)cosine45deg * 2.0f, 0);
	glm::ivec3 coordinates(floor(_.x), floor(_.y), _.z);

	coordinates.x -= 50;
	coordinates.y += 50;
	coordinates.z += 49;
	for(unsigned int i = 0; i < 50 && this->getTile(coordinates) == 0; i++) {
		coordinates.x += 1;
		coordinates.y -= 1;
		coordinates.z -= 1;
	}

	return coordinates;
}

void ChunkContainer::onBind(string &bind, binds::Action action) {
	if(bind == "chunk.mouseSelectTile" && action == binds::PRESS) {
		this->selectTile(this->findCandidateSelectedTile(engine->camera->mouseToWorld(engine->mouse)), false);
	}
	else if(bind == "chunk.mouseRightClickTile" && action == binds::PRESS) {
		this->rightClickTile(this->findCandidateSelectedTile(engine->camera->mouseToWorld(engine->mouse)));
	}
	else if(bind == "chunk.selectTile" && action == binds::PRESS) {
		this->selectTile(this->tileSelectionSprite->getPosition(), false);
	}
}

void ChunkContainer::onAxis(string &bind, double value) {
	if((bind == "chunk.xAxis" || bind == "chunk.yAxis") && value != 0.0) {
		glm::vec2 position = engine->camera->getPosition();
		position.x += 0.5;
		position.y = -position.y;
		this->selectTile(this->findCandidateSelectedTile(position), true);
	}
	else if(bind == "chunk.mouseXAxis" || bind == "chunk.mouseYAxis") {
		this->hoverTile(this->findCandidateSelectedTile(engine->camera->mouseToWorld(engine->mouse)));
	}
}

void ChunkContainer::commit() {
	if(this->tileSelectionSprite == nullptr) {
		this->tileSelectionSprite = (new OverlappingTile(this))
			->setTexture(18)
			->setPosition(glm::uvec3(0, 0, 0))
			->setZIndex(1);
	}

	if(this->characterSelectionSprite == nullptr) {
		this->characterSelectionSprite = (new OverlappingTile(this))
			->setTexture(18)
			->setColor(glm::vec4(0.0, 0.55, 1.0, 1.0))
			->setPosition(glm::uvec3(0, 0, 0))
			->setZIndex(100);
	}
}

Team* ChunkContainer::getPlayerTeam() {
	return this->playerTeam;
}

void es::defineChunkContainer() {
	esRegisterNamespace(engine->eggscript, "ChunkContainer");
	esNamespaceInherit(engine->eggscript, "SimObject", "ChunkContainer");

	esEntryType getCharacterArguments[2] = {ES_ENTRY_OBJECT, ES_ENTRY_MATRIX};
	esRegisterMethod(engine->eggscript, ES_ENTRY_OBJECT, es::ChunkContainer__getCharacter, "ChunkContainer", "getCharacter", 2, getCharacterArguments);

	esEntryType selectCharacterArguments[2] = {ES_ENTRY_OBJECT, ES_ENTRY_OBJECT};
	esRegisterMethod(engine->eggscript, ES_ENTRY_INVALID, es::ChunkContainer__selectCharacter, "ChunkContainer", "selectCharacter", 2, selectCharacterArguments);

	esEntryType getPlayerTeamArguments[1] = {ES_ENTRY_OBJECT};
	esRegisterMethod(engine->eggscript, ES_ENTRY_OBJECT, es::ChunkContainer__getPlayerTeam, "ChunkContainer", "getPlayerTeam", 1, getPlayerTeamArguments);

	esRegisterMethod(engine->eggscript, ES_ENTRY_OBJECT, es::ChunkContainer__getSelectedCharacter, "ChunkContainer", "getSelectedCharacter", 1, getPlayerTeamArguments);

	esRegisterFunction(engine->eggscript, ES_ENTRY_OBJECT, es::getChunkContainer, "getChunkContainer", 0, nullptr);

	esEntryType setTileArguments[3] = {ES_ENTRY_OBJECT, ES_ENTRY_MATRIX, ES_ENTRY_NUMBER};
	esRegisterMethod(engine->eggscript, ES_ENTRY_INVALID, es::ChunkContainer__setTile, "ChunkContainer", "setTile", 3, setTileArguments);

	esEntryType tileToScreenArguments[1] = {ES_ENTRY_MATRIX};
	esRegisterFunction(engine->eggscript, ES_ENTRY_OBJECT, es::tileToScreen, "tileToScreen", 1, tileToScreenArguments);
}

esEntryPtr es::getChunkContainer(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(engine->chunkContainer != nullptr) {
		return esCreateObject(engine->chunkContainer->reference);
	}
	return nullptr;
}

esEntryPtr es::ChunkContainer__getCharacter(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 2 && esCompareNamespaceToObject(args[0].objectData, "ChunkContainer") && args[1].matrixData->rows == 3 && args[1].matrixData->columns == 1) {
		ChunkContainer* container = (ChunkContainer*)args[0].objectData->objectWrapper->data;
		glm::uvec3 position(
			args[1].matrixData->data[0][0].numberData,
			args[1].matrixData->data[1][0].numberData,
			args[1].matrixData->data[2][0].numberData
		);

		auto found = container->positionToCharacter.find(position);
		if(found != container->positionToCharacter.end()) {
			return esCreateObject(found.value()->reference);
		}
	}
	return nullptr;
}

esEntryPtr es::ChunkContainer__selectCharacter(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 2 && esCompareNamespaceToObject(args[0].objectData, "ChunkContainer") && esCompareNamespaceToObjectParents(args[1].objectData, "Character")) {
		ChunkContainer* container = (ChunkContainer*)args[0].objectData->objectWrapper->data;
		Character* character = (Character*)args[1].objectData->objectWrapper->data;
		container->selectCharacter(character);
	}
	return nullptr;
}

esEntryPtr es::ChunkContainer__getPlayerTeam(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 1 && esCompareNamespaceToObject(args[0].objectData, "ChunkContainer")) {
		ChunkContainer* container = (ChunkContainer*)args[0].objectData->objectWrapper->data;
		return esCreateObject(container->getPlayerTeam()->reference);
	}
	return nullptr;
}

esEntryPtr es::ChunkContainer__getSelectedCharacter(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 1 && esCompareNamespaceToObject(args[0].objectData, "ChunkContainer")) {
		ChunkContainer* container = (ChunkContainer*)args[0].objectData->objectWrapper->data;
		if(container->selectedCharacter == nullptr) {
			return nullptr;
		}
		return esCreateObject(container->selectedCharacter->reference);
	}
	return nullptr;
}

esEntryPtr es::ChunkContainer__setTile(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 3 && esCompareNamespaceToObject(args[0].objectData, "ChunkContainer") && args[1].matrixData->rows == 3 && args[1].matrixData->columns == 1) {
		ChunkContainer* container = (ChunkContainer*)args[0].objectData->objectWrapper->data;
		glm::ivec3 position(
			args[1].matrixData->data[0][0].numberData,
			args[1].matrixData->data[1][0].numberData,
			args[1].matrixData->data[2][0].numberData
		);

		container->setTile(position, args[2].numberData);
	}
	return nullptr;
}

esEntryPtr es::tileToScreen(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(args[0].matrixData->rows == 3 && args[0].matrixData->columns == 1) {
		glm::vec2 position = tilemath::tileToScreen(glm::vec3(
			args[0].matrixData->data[0][0].numberData,
			args[0].matrixData->data[1][0].numberData,
			args[0].matrixData->data[2][0].numberData
		));
		return esCreateVector(2, (double)position.x, (double)position.y);
	}
	return nullptr;
}
