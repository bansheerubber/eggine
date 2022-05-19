#include "chunkContainer.h"

#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <math.h>

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

#include "../basic/camera.h"
#include "character.h"
#include "chunk.h"
#include "../engine/console.h"
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
render::VertexBuffer* ChunkContainer::XRay = nullptr;

void initChunk(class ChunkContainer* container, class Chunk** chunk) {
	*chunk = nullptr;
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
			engine->manager->carton->database.get()->equals("extension", ".ss")->exec()
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

	if(ChunkContainer::Colors == nullptr) { // colors for square
		ChunkContainer::Colors = new render::VertexBuffer(&engine->renderWindow);
		ChunkContainer::Colors->setData((glm::vec4*)&ChunkContainer::ColorsSource[0], sizeof(ChunkContainer::ColorsSource), alignof(glm::vec4));
	}

	if(ChunkContainer::XRay == nullptr) {
		ChunkContainer::XRay = new render::VertexBuffer(&engine->renderWindow);
		ChunkContainer::XRay->setData((int*)&ChunkContainer::XRaySource, sizeof(ChunkContainer::XRaySource), alignof(int));
	}

	this->renderOrder.allocate(8);

	// create torquescript object
	this->reference = esInstantiateObject(engine->eggscript, "ChunkContainer", this);

	this->playerTeam = new Team();
	this->playerTeam->name = "Player";

	this->enemyTeam = new Team();
	this->enemyTeam->name = "Alien";
}

ChunkContainer::~ChunkContainer() {
	for(uint64_t i = 0; i < this->renderOrder.head; i++) {
		delete this->renderOrder[i];
	}
}

void ChunkContainer::setSize(unsigned int size) {
	this->renderOrder.allocate(size * size);
	this->renderOrder.head = size * size;
	this->size = size;
}

unsigned int ChunkContainer::getSize() {
	return this->size;
}

void ChunkContainer::setRotation(tilemath::Rotation rotation) {
	glm::vec2 camera = engine->camera->getPosition();
	camera.x += 0.5;
	camera.y = -camera.y;
	glm::vec3 position = this->screenToTile(camera);
	
	this->rotation = rotation;

	engine->camera->setPosition(tilemath::tileToScreen(position, Chunk::Size * Chunk::Size * this->getSize(), rotation));

	Chunk::BuildOffsets(rotation);

	// move the chunks into a new render order
	Chunk** newOrder = new Chunk*[this->renderOrder.head];
	for(unsigned int i = 0; i < this->renderOrder.head; i++) {
		Chunk* chunk = this->renderOrder[i];
		unsigned int index = tilemath::coordinateToIndex(chunk->getPosition(), this->size, this->getRotation());
		newOrder[index] = chunk;
		chunk->setPosition(tilemath::indexToCoordinate(index, this->size, this->getRotation()));
	}

	// copy new order into current render order
	for(unsigned int i = 0; i < this->renderOrder.head; i++) {
		this->renderOrder[i] = newOrder[i];
		newOrder[i]->updateRotation(rotation);
	}

	delete[] newOrder;
}

tilemath::Rotation ChunkContainer::getRotation() {
	return this->rotation;
}

Chunk* ChunkContainer::addChunk(glm::uvec2 position) {
	int64_t index = tilemath::coordinateToIndex(position, this->size, this->getRotation());
	if(this->renderOrder[index] == nullptr) {
		this->renderOrder[index] = new Chunk(this);
	}
	this->renderOrder[index]->setPosition(position);
	return this->renderOrder[index];
}

Chunk* ChunkContainer::getChunk(uint64_t index) {
	if(index >= this->size * this->size) {
		console::error("ChunkContainer::getChunk(): chunk index out of bounds\n");
		exit(1);
	}

	return this->renderOrder[index];
}

uint64_t ChunkContainer::getChunkCount() {
	return this->renderOrder.head;
}

void ChunkContainer::render(double deltaTime, RenderContext &context) {
	this->timer++;
	
	engine->renderWindow.getState(0).bindProgram(ChunkContainer::Program);
	engine->renderWindow.getState(0).bindTexture("spriteTexture", ChunkContainer::Image->texture);

	#ifdef EGGINE_DEBUG
	uint64_t chunksRendered = 0;
	uint64_t tilesRendered = 0;
	uint64_t tiles = 0;
	uint64_t drawCalls = 0;
	uint64_t overlappingCalls = 0;
	#endif

	// normal rendering
	for(uint64_t i = 0; i < this->renderOrder.head; i++) {
		Chunk* chunk = this->renderOrder[i];
		chunk->renderChunk(deltaTime, context);

		#ifdef EGGINE_DEBUG
		if(!chunk->isCulled) {
			chunksRendered++;
			tilesRendered += Chunk::Size * Chunk::Size * chunk->height;
			drawCalls += chunk->drawCalls;
			overlappingCalls += chunk->layers.size();
		}
		
		tiles += Chunk::Size * Chunk::Size * chunk->height;
		#endif
	}

	// re-draw xray tiles
	for(uint64_t i = 0; i < this->renderOrder.head; i++) {
		this->renderOrder[i]->renderXRay(deltaTime, context);
	}

	#ifdef EGGINE_DEBUG
	engine->debug.addInfoMessage(fmt::format("{}/{} chunks rendered", chunksRendered, this->renderOrder.head));
	engine->debug.addInfoMessage(fmt::format("{}/{} tiles rendered", tilesRendered, tiles));
	engine->debug.addInfoMessage(fmt::format("{} chunk draw calls, {} overlapping calls", drawCalls, overlappingCalls));
	#endif
}

bool ChunkContainer::isValidTilePosition(glm::uvec3 position) {
	if(
		position.x >= this->size * Chunk::Size 
		|| position.y >= this->size * Chunk::Size
		|| position.z >= 50
	) {
		return false;
	}
	return true;
}

void ChunkContainer::selectCharacter(Character* character) {
	this->selectedCharacter = character;

	if(character != nullptr) {
		glm::uvec3 position = character->getPosition();
		if(position.z != 0) {
			position.z -= 1;
		}

		this->characterSelectionSprite->setPosition(position);
	}
}

Character* ChunkContainer::getSelectedCharacter() {
	return this->selectedCharacter;
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
	int64_t index = tilemath::coordinateToIndex(chunkPosition, this->size, this->getRotation());
	this->renderOrder[index]->setTileTexture(position, texture);
}

int ChunkContainer::getTile(glm::ivec3 position) {
	if(!this->isValidTilePosition(position)) {
		return 0;
	}

	glm::uvec2 chunkPosition = glm::uvec3(position) / (unsigned int)Chunk::Size;
	int64_t index = tilemath::coordinateToIndex(chunkPosition, this->size, this->getRotation());
	return this->renderOrder[index]->getTileTexture(position);
}

resources::SpriteSheetInfo ChunkContainer::getSpriteInfo(glm::ivec3 position, bool original) {
	int texture = this->getTile(position);
	if(original) {
		resources::SpriteSheetInfo info = ChunkContainer::Image->getSpriteInfo(texture);
		if(info.facing == resources::FACING_INVALID) {
			return info;
		}
		else {
			return ChunkContainer::Image->getSpriteInfo(
				info.facingsMap->rotateFacing(info.facing, this->getRotation(), tilemath::ROTATION_0_DEG)
			);
		}
	}
	else {
		return ChunkContainer::Image->getSpriteInfo(texture);
	}
}

void ChunkContainer::selectTile(glm::ivec3 position, bool browsing, bool controller) {
	if(!this->isValidTilePosition(position)) {
		return;
	}

	esEntry arguments[3];
	esCreateVectorAt(&arguments[0], 3, (double)position.x, (double)position.y, (double)position.z);
	esCreateNumberAt(&arguments[1], browsing);
	esCreateNumberAt(&arguments[2], controller);
	esDeleteEntry(esCallFunction(engine->eggscript, "onSelectTile", 3, arguments));
}

TileNeighborIterator ChunkContainer::getNeighbors(glm::ivec3 position) {
	return TileNeighborIterator(position);
}

void ChunkContainer::rightClickTile(glm::ivec3 position) {
	esEntry arguments[1];
	esCreateVectorAt(&arguments[0], 3, (double)position.x, (double)position.y, (double)position.z);
	esDeleteEntry(esCallFunction(engine->eggscript, "onRightClickTile", 1, arguments));
}

void ChunkContainer::hoverTile(glm::ivec3 position) {
	esEntry arguments[1];
	esCreateVectorAt(&arguments[0], 3, (double)position.x, (double)position.y, (double)position.z);
	esDeleteEntry(esCallFunction(engine->eggscript, "onHoverTile", 1, arguments));
}

glm::vec3 ChunkContainer::screenToTile(glm::vec2 screen) {
	double cosine45deg = cos(M_PI / 4.0f);
	glm::mat2 inverseBasis = glm::mat2(
		cosine45deg, cosine45deg,
		-cosine45deg * 2.0f, cosine45deg * 2.0f
	);
	glm::vec3 _((inverseBasis * screen) * (float)cosine45deg * 2.0f - glm::vec2(-1, 1), 0);
	glm::vec3 coordinates(0, 0, 0);
	glm::vec3 directionTowardsCamera(tilemath::directionTowardsCamera(this->getRotation()), 1);
	switch(this->getRotation()) {
		case tilemath::ROTATION_0_DEG: {
			coordinates = glm::vec3(_.x, _.y, _.z);
			break;
		}

		case tilemath::ROTATION_90_DEG: {
			coordinates = glm::vec3(-_.y + 1.0, _.x, _.z);
			break;
		}

		case tilemath::ROTATION_180_DEG: {
			coordinates = glm::vec3(-_.x + 1.0, -_.y + 1.0, _.z);
			break;
		}

		case tilemath::ROTATION_270_DEG: {
			coordinates = glm::vec3(_.y, -_.x + 1.0, _.z);
			break;
		}
	}

	coordinates.x += directionTowardsCamera.x;
	coordinates.y += directionTowardsCamera.y;

	return coordinates;
}

glm::ivec3 ChunkContainer::findCandidateSelectedTile(glm::vec2 screen) {
	return glm::ivec3(screenToTile(screen));
}

void ChunkContainer::onBind(string &bind, binds::Action action) {
	if(bind == "chunk.mouseSelectTile" && action == binds::RELEASE) {
		this->selectTile(this->findCandidateSelectedTile(engine->camera->mouseToWorld(engine->mouse)), false, false);
	}
	else if(bind == "chunk.mouseRightClickTile" && action == binds::RELEASE) {
		this->rightClickTile(this->findCandidateSelectedTile(engine->camera->mouseToWorld(engine->mouse)));
	}
	else if(bind == "chunk.selectTile" && action == binds::PRESS) {
		glm::vec2 position = engine->camera->getPosition();
		position.x += 0.5;
		position.y = -position.y;
		this->selectTile(this->findCandidateSelectedTile(position), false, true);
	}
}

void ChunkContainer::onAxis(string &bind, double value) {
	if((bind == "chunk.xAxis" || bind == "chunk.yAxis") && value != 0.0) {
		glm::vec2 position = engine->camera->getPosition();
		position.x += 0.5;
		position.y = -position.y;
		this->selectTile(this->findCandidateSelectedTile(position), true, true);
	}
	else if(bind == "chunk.mouseXAxis" || bind == "chunk.mouseYAxis") {
		this->hoverTile(this->findCandidateSelectedTile(engine->camera->mouseToWorld(engine->mouse)));
	}
}

void ChunkContainer::commit() {
	if(this->tileSelectionSprite == nullptr) {
		this->tileSelectionSprite = (new OverlappingTile())
			->setTexture(18)
			->setPosition(glm::uvec3(0, 0, 0))
			->setZIndex(1)
			->setXRay(2)
			->setColor(glm::vec4(0, 0, 0, 0));
	}

	if(this->characterSelectionSprite == nullptr) {
		this->characterSelectionSprite = (new OverlappingTile())
			->setTexture(18)
			->setColor(glm::vec4(0.0, 0.55, 1.0, 1.0))
			->setPosition(glm::uvec3(0, 0, 0))
			->setZIndex(100)
			->setXRay(2);
	}
}

void ChunkContainer::setPlayerTeam(Team* team) {
	this->playerTeam = team;
}

Team* ChunkContainer::getPlayerTeam() {
	return this->playerTeam;
}

void ChunkContainer::setEnemyTeam(Team* team) {
	this->enemyTeam = team;
}

Team* ChunkContainer::getEnemyTeam() {
	return this->enemyTeam;
}

void es::defineChunkContainer() {
	esRegisterNamespace(engine->eggscript, "ChunkContainer");
	esNamespaceInherit(engine->eggscript, "SimObject", "ChunkContainer");

	esEntryType getCharacterArguments[2] = {ES_ENTRY_OBJECT, ES_ENTRY_MATRIX};
	esRegisterMethod(engine->eggscript, ES_ENTRY_OBJECT, es::ChunkContainer__getCharacter, "ChunkContainer", "getCharacter", 2, getCharacterArguments);

	esEntryType selectCharacterArguments[2] = {ES_ENTRY_OBJECT, ES_ENTRY_OBJECT};
	esRegisterMethod(engine->eggscript, ES_ENTRY_EMPTY, es::ChunkContainer__selectCharacter, "ChunkContainer", "selectCharacter", 2, selectCharacterArguments);

	esEntryType getPlayerTeamArguments[1] = {ES_ENTRY_OBJECT};
	esRegisterMethod(engine->eggscript, ES_ENTRY_OBJECT, es::ChunkContainer__getPlayerTeam, "ChunkContainer", "getPlayerTeam", 1, getPlayerTeamArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_OBJECT, es::ChunkContainer__getEnemyTeam, "ChunkContainer", "getEnemyTeam", 1, getPlayerTeamArguments);

	esRegisterMethod(engine->eggscript, ES_ENTRY_INVALID, es::ChunkContainer__setPlayerTeam, "ChunkContainer", "setPlayerTeam", 2, selectCharacterArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_INVALID, es::ChunkContainer__setEnemyTeam, "ChunkContainer", "setEnemyTeam", 2, selectCharacterArguments);

	esRegisterMethod(engine->eggscript, ES_ENTRY_OBJECT, es::ChunkContainer__getSelectedCharacter, "ChunkContainer", "getSelectedCharacter", 1, getPlayerTeamArguments);

	esRegisterFunction(engine->eggscript, ES_ENTRY_OBJECT, es::getChunkContainer, "getChunkContainer", 0, nullptr);

	esEntryType setTileArguments[3] = {ES_ENTRY_OBJECT, ES_ENTRY_MATRIX, ES_ENTRY_NUMBER};
	esRegisterMethod(engine->eggscript, ES_ENTRY_EMPTY, es::ChunkContainer__setTile, "ChunkContainer", "setTile", 3, setTileArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_NUMBER, es::ChunkContainer__getTile, "ChunkContainer", "getTile", 2, getCharacterArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_NUMBER, es::ChunkContainer__getWall, "ChunkContainer", "getWall", 2, getCharacterArguments);

	esEntryType setRotationArguments[2] = {ES_ENTRY_OBJECT, ES_ENTRY_NUMBER};
	esRegisterMethod(engine->eggscript, ES_ENTRY_EMPTY, es::ChunkContainer__setRotation, "ChunkContainer", "setRotation", 2, setRotationArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_NUMBER, es::ChunkContainer__getRotation, "ChunkContainer", "getRotation", 1, getPlayerTeamArguments);

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
	if(argc == 2 && esCompareNamespaceToObject(args[0].objectData, "ChunkContainer")) {
		ChunkContainer* container = (ChunkContainer*)args[0].objectData->objectWrapper->data;

		if(esCompareNamespaceToObjectParents(args[1].objectData, "Character")) {
			Character* character = (Character*)args[1].objectData->objectWrapper->data;
			container->selectCharacter(character);
		}
		else {
			container->selectCharacter(nullptr);
		}
	}
	return nullptr;
}

esEntryPtr es::ChunkContainer__setPlayerTeam(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 2 && esCompareNamespaceToObject(args[0].objectData, "ChunkContainer") && esCompareNamespaceToObject(args[1].objectData, "Team")) {
		((ChunkContainer*)args[0].objectData->objectWrapper->data)->setPlayerTeam((Team*)args[1].objectData->objectWrapper->data);
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

esEntryPtr es::ChunkContainer__setEnemyTeam(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 2 && esCompareNamespaceToObject(args[0].objectData, "ChunkContainer") && esCompareNamespaceToObject(args[1].objectData, "Team")) {
		((ChunkContainer*)args[0].objectData->objectWrapper->data)->setEnemyTeam((Team*)args[1].objectData->objectWrapper->data);
	}
	return nullptr;
}

esEntryPtr es::ChunkContainer__getEnemyTeam(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 1 && esCompareNamespaceToObject(args[0].objectData, "ChunkContainer")) {
		ChunkContainer* container = (ChunkContainer*)args[0].objectData->objectWrapper->data;
		return esCreateObject(container->getEnemyTeam()->reference);
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

esEntryPtr es::ChunkContainer__getTile(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 2 && esCompareNamespaceToObject(args[0].objectData, "ChunkContainer") && args[1].matrixData->rows == 3 && args[1].matrixData->columns == 1) {
		ChunkContainer* container = (ChunkContainer*)args[0].objectData->objectWrapper->data;
		glm::ivec3 position(
			args[1].matrixData->data[0][0].numberData,
			args[1].matrixData->data[1][0].numberData,
			args[1].matrixData->data[2][0].numberData
		);

		return esCreateNumber(container->getTile(position));
	}
	return nullptr;
}

esEntryPtr es::ChunkContainer__getWall(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 2 && esCompareNamespaceToObject(args[0].objectData, "ChunkContainer") && args[1].matrixData->rows == 3 && args[1].matrixData->columns == 1) {
		ChunkContainer* container = (ChunkContainer*)args[0].objectData->objectWrapper->data;
		glm::ivec3 position(
			args[1].matrixData->data[0][0].numberData,
			args[1].matrixData->data[1][0].numberData,
			args[1].matrixData->data[2][0].numberData
		);

		return esCreateNumber(container->getSpriteInfo(position).wall);
	}
	return nullptr;
}

esEntryPtr es::ChunkContainer__setRotation(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 2 && esCompareNamespaceToObject(args[0].objectData, "ChunkContainer")) {
		ChunkContainer* container = (ChunkContainer*)args[0].objectData->objectWrapper->data;
		container->setRotation((tilemath::Rotation)(unsigned int)args[1].numberData);
	}
	return nullptr;
}

esEntryPtr es::ChunkContainer__getRotation(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 1 && esCompareNamespaceToObject(args[0].objectData, "ChunkContainer")) {
		ChunkContainer* container = (ChunkContainer*)args[0].objectData->objectWrapper->data;
		return esCreateNumber(container->getRotation());
	}
	return nullptr;
}

esEntryPtr es::tileToScreen(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(args[0].matrixData->rows == 3 && args[0].matrixData->columns == 1) {
		glm::vec2 position = tilemath::tileToScreen(
			glm::vec3(
				args[0].matrixData->data[0][0].numberData,
				args[0].matrixData->data[1][0].numberData,
				args[0].matrixData->data[2][0].numberData
			),
			Chunk::Size * engine->chunkContainer->getSize(),
			engine->chunkContainer->getRotation()
		);
		return esCreateVector(2, (double)position.x, (double)position.y);
	}
	return nullptr;
}
