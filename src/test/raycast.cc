#include "raycast.h"

#include "chunkContainer.h"
#include "../engine/engine.h"
#include "../util/sign.h"

RaycastMarcher::RaycastMarcher(glm::ivec3 start, glm::vec3 direction, unsigned int length, unsigned int options) {
	if(isnan(glm::length(direction)) || glm::length(direction) < 0.99) {
		this->_finished = true;
		this->currentNormal = glm::vec3();
		return;
	}
	
	this->start = start;
	this->length = length;
	this->options = options;
	this->useEnd = false;
	this->position = glm::ivec3(start);
	this->steps = glm::ivec3(sign(direction.x), sign(direction.y), sign(direction.z));
	this->bounds = glm::vec3(0.5 / abs(direction.x), 0.5 / abs(direction.y), 0.5 / abs(direction.z));
	this->delta = glm::vec3(steps.x / direction.x, steps.y / direction.y, steps.z / direction.z);
}

RaycastMarcher::RaycastMarcher(glm::ivec3 start, glm::ivec3 end, unsigned int options) {
	glm::vec3 direction = end - start;
	direction = glm::normalize(direction);
	if(isnan(glm::length(direction)) || glm::length(direction) < 0.99) {
		this->_finished = true;
		this->currentNormal = glm::vec3();
		return;
	}
	
	this->start = start;
	this->end = end;
	this->options = options;
	this->useEnd = true;
	this->position = glm::ivec3(start);
	this->steps = glm::ivec3(sign(direction.x), sign(direction.y), sign(direction.z));
	this->bounds = glm::vec3(0.5 / abs(direction.x), 0.5 / abs(direction.y), 0.5 / abs(direction.z));
	this->delta = glm::vec3(steps.x / direction.x, steps.y / direction.y, steps.z / direction.z);
}

glm::ivec3 RaycastMarcher::value() {
	return this->position;
}

bool RaycastMarcher::finished() {
	return this->_finished;
}

std::vector<RaycastResult>& RaycastMarcher::cast() {
	for(; !this->finished(); ++(*this));
	return this->results;
}

RaycastMarcher& RaycastMarcher::operator++() {
	if(this->finished()) {
		return *this;
	}

	const glm::ivec3 offset(0, 0, -1);

	if(!engine->chunkContainer->isValidTilePosition(this->position)) {
		this->_finished = true;
		this->currentNormal = glm::vec3();
		return *this;
	}
	else if(engine->chunkContainer->getTile(this->position) != 0) {
		if(!(this->options & RAYCAST_PENETRATE)) {
			this->_finished = true;
		}

		this->results.push_back({
			position: position,
			normal: this->currentNormal,
		});
		return *this;
	}
	else if(
		engine->chunkContainer->isValidTilePosition(this->position + offset)
		&& engine->chunkContainer->getSpriteInfo(this->position).wall != resources::NO_WALL
		&& engine->chunkContainer->getTile(this->position + offset) != 0
	) { // check for walls
		if(!(this->options & RAYCAST_PENETRATE)) {
			this->_finished = true;
		}

		this->results.push_back({
			position: position + offset,
			normal: this->currentNormal,
		});
		return *this;
	}
	else if(this->useEnd && this->position == this->end) {
		this->_finished = true;
		this->currentNormal = glm::vec3();
		return *this;
	}
	else if(!this->useEnd && this->length < glm::distance(glm::vec3(this->start), glm::vec3(this->position))) {
		this->_finished = true;
		this->currentNormal = glm::vec3();
		return *this;
	}

	if(this->bounds.x < this->bounds.y && this->bounds.x < this->bounds.z) {
		this->position.x += this->steps.x;
		this->bounds.x += this->delta.x;
		this->currentNormal = glm::vec3(-this->steps.x, 0, 0);
	}
	else if(this->bounds.y <= this->bounds.x && this->bounds.y < this->bounds.z) {
		this->position.y += this->steps.y;
		this->bounds.y += this->delta.y;
		this->currentNormal = glm::vec3(0, -this->steps.y, 0);
	}
	else {
		this->position.z += this->steps.z;
		this->bounds.z += this->delta.z;
		this->currentNormal = glm::vec3(0, 0, -this->steps.z);
	}

	return *this;
}

std::vector<RaycastResult> raycast(glm::ivec3 start, glm::vec3 direction, unsigned int length, unsigned int options) {
	RaycastMarcher marcher(start, direction, length, options);
	return marcher.cast();
}

std::vector<RaycastResult> raycast(glm::ivec3 start, glm::ivec3 end, unsigned int options) {
	RaycastMarcher marcher(start, end, options);
	return marcher.cast();
}

void es::defineRaycast() {
	esEntryType raycastArgs[3] = { ES_ENTRY_MATRIX, ES_ENTRY_MATRIX, ES_ENTRY_NUMBER };
	esRegisterFunction(engine->eggscript, ES_ENTRY_OBJECT, es::raycast, "raycast", 3, raycastArgs);
}

esEntryPtr es::raycast(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(
		argc >= 2
		&& args[0].matrixData->rows == 3 && args[0].matrixData->columns == 1
		&& args[1].matrixData->rows == 3 && args[1].matrixData->columns == 1
	) {
		std::vector<RaycastResult> result = ::raycast(
			glm::ivec3(
				args[0].matrixData->data[0][0].numberData,
				args[0].matrixData->data[1][0].numberData,
				args[0].matrixData->data[2][0].numberData
			),
			glm::ivec3(
				args[1].matrixData->data[0][0].numberData,
				args[1].matrixData->data[1][0].numberData,
				args[1].matrixData->data[2][0].numberData
			),
			(unsigned int)args[2].numberData
		);
		
		if(result.size() == 0) {
			return nullptr;
		}

		esObjectReferencePtr array = esCreateArray(engine->eggscript);
		esArrayPush(array, esCreateVector(3, (double)result[0].position.x, (double)result[0].position.y, (double)result[0].position.z));
		return esCreateObject(array);
	}
	return nullptr;
}
