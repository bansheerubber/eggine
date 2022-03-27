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

	// initialize direction/normal
	if(this->bounds.x < this->bounds.y && this->bounds.x < this->bounds.z) {
		this->currentNormal = glm::vec3(-this->steps.x, 0, 0);
		
		if(this->steps.x < 0) {
			this->direction = DIRECTION_WEST;
		}
		else if(this->steps.x > 0) {
			this->direction = DIRECTION_EAST;
		}
	}
	else if(this->bounds.y <= this->bounds.x && this->bounds.y < this->bounds.z) {
		this->currentNormal = glm::vec3(0, -this->steps.y, 0);

		if(this->steps.y < 0) {
			this->direction = DIRECTION_NORTH;
		}
		else if(this->steps.y > 0) {
			this->direction = DIRECTION_SOUTH;
		}
	}
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

	const NeighborDirection invalidDirectionsForWall[8][2] = {
		{DIRECTION_SOUTH, INVALID_DIRECTION}, // north-facing wall
		{DIRECTION_WEST, INVALID_DIRECTION}, // east-facing wall
		{DIRECTION_NORTH, INVALID_DIRECTION}, // south-facing wall
		{DIRECTION_EAST, INVALID_DIRECTION}, // west-facing wall
		{DIRECTION_SOUTH, DIRECTION_WEST}, // north-east corner wall
		{DIRECTION_NORTH, DIRECTION_WEST}, // south-east corner wall
		{DIRECTION_NORTH, DIRECTION_EAST}, // south-west corner wall
		{DIRECTION_SOUTH, DIRECTION_EAST}, // north-west corner wall
	};

	if(!engine->chunkContainer->isValidTilePosition(this->position)) {
		this->_finished = true;
		this->currentNormal = glm::vec3();
		return *this;
	}

	this->oldDirection = this->direction;
	// determine heading before we do any tile checks
	if(this->bounds.x < this->bounds.y && this->bounds.x < this->bounds.z) {
		this->currentNormal = glm::vec3(-this->steps.x, 0, 0);
		
		if(this->steps.x < 0) {
			this->direction = DIRECTION_WEST;
		}
		else if(this->steps.x > 0) {
			this->direction = DIRECTION_EAST;
		}
	}
	else if(this->bounds.y <= this->bounds.x && this->bounds.y < this->bounds.z) {
		this->currentNormal = glm::vec3(0, -this->steps.y, 0);

		if(this->steps.y < 0) {
			this->direction = DIRECTION_NORTH;
		}
		else if(this->steps.y > 0) {
			this->direction = DIRECTION_SOUTH;
		}
	}
	else {
		this->currentNormal = glm::vec3(0, 0, -this->steps.z);

		if(this->steps.z < 0) {
			this->direction = DIRECTION_DOWN;
		}
		else if(this->steps.z > 0) {
			this->direction = DIRECTION_UP;
		}
	}

	// check position
	{
		resources::SpriteSheetInfo info;
		if(this->currentHits.find(this->position) != this->currentHits.end()) {
			goto normal_skip;
		}
		
		info = engine->chunkContainer->getSpriteInfo(this->position, true);

		if(info.index == 0) {
			goto normal_skip;
		}

		if(info.wall == 0) {
			if(!(this->options & RAYCAST_PENETRATE)) {
				this->_finished = true;
			}

			this->results.push_back({
				position: this->position,
				normal: this->currentNormal,
			});

			this->currentHits.insert(this->position);

			if(!(this->options & RAYCAST_PENETRATE)) {
				return *this;
			}
		}
		else {
			for(unsigned int i = 0; i < 2; i++) {
				char index = info.wall - 1;
				if(
					invalidDirectionsForWall[(unsigned char)index][i] == this->oldDirection || (
						invalidDirectionsForWall[(unsigned char)index][i] == flipDirection(this->direction)
						&& !this->canFinish()
					)
				) {
					if(!(this->options & RAYCAST_PENETRATE)) {
						this->_finished = true;
					}

					this->results.push_back({
						position: this->position,
						normal: this->currentNormal,
					});

					this->currentHits.insert(this->position);

					if(!(this->options & RAYCAST_PENETRATE)) {
						return *this;
					}
				}
			}
		}
	}
	normal_skip:

	// check position below current position (special double-height tile case)
	{
		const glm::ivec3 offset(0, 0, -1);
		resources::SpriteSheetInfo info;
		if(
			this->currentHits.find(this->position + offset) != this->currentHits.end()
			|| !engine->chunkContainer->isValidTilePosition(this->position + offset)
		) {
			goto wall_skip;
		}
		
		info = engine->chunkContainer->getSpriteInfo(this->position + offset, true);

		if(info.wall == 0) {
			goto wall_skip;
		}

		for(unsigned int i = 0; i < 2; i++) {
			char index = info.wall - 1;
			if(
				invalidDirectionsForWall[(unsigned char)index][i] == this->oldDirection || (
					invalidDirectionsForWall[(unsigned char)index][i] == flipDirection(this->direction)
					&& !this->canFinish()
				)
			) {
				if(!(this->options & RAYCAST_PENETRATE)) {
					this->_finished = true;
				}

				this->results.push_back({
					position: this->position,
					normal: this->currentNormal,
				});

				this->currentHits.insert(this->position);

				if(!(this->options & RAYCAST_PENETRATE)) {
					return *this;
				}
			}
		}
	}
	wall_skip:
	
	if(this->canFinish()) {
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

bool RaycastMarcher::canFinish() {
	return (this->useEnd && this->position == this->end) || (!this->useEnd && this->length < glm::distance(glm::vec3(this->start), glm::vec3(this->position)));
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
		for(size_t i = 0; i < result.size(); i++) {
			esArrayPush(array, esCreateVector(3, (double)result[i].position.x, (double)result[i].position.y, (double)result[i].position.z));
		}
		
		return esCreateObject(array);
	}
	return nullptr;
}
