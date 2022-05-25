#include "quadraticRaycast.h"

#include "chunkContainer.h"
#include "../engine/engine.h"
#include "../util/sign.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

double calculateProjectileAngle(glm::vec3 start, glm::vec3 end, double speed) {
	double distance = glm::distance(glm::vec2(start.x, start.y), glm::vec2(end.x, end.y));
	double height = start.z - end.z;
	double gravity = -9.8;
	return M_PI - atan2(
		pow(speed, 2) + sqrt(pow(speed, 4) - gravity * (gravity * pow(distance, 2) + 2 * height * pow(speed, 2))),
		gravity * distance
	);
}

QuadraticRaycastMarcher::QuadraticRaycastMarcher(glm::ivec3 start, glm::ivec3 end, double speed, unsigned int options) {
	this->angle = calculateProjectileAngle(start, end, speed);
	this->start = start;
	this->end = end;
	this->speed = speed;

	this->direction = glm::normalize(glm::vec2(end) - glm::vec2(start));
	this->steps = glm::ivec2(sign(direction.x), sign(direction.y));
	this->bounds = glm::vec2(0.5 / abs(direction.x), 0.5 / abs(direction.y));
	this->delta = glm::vec2(this->steps.x / this->direction.x, this->steps.y / this->direction.y);

	this->options = options;

	this->position2d = glm::ivec2(start);
	this->position = start;
	this->lastPosition = start;

	if(std::isnan(this->angle)) {
		this->_finished = true;
	}
}

std::vector<RaycastResult> &QuadraticRaycastMarcher::cast() {
	for(; !this->finished(); ++(*this));
	return this->results;
}

double round(double input, bool floorOrCeil) {
	if(floorOrCeil) {
		return floor(input);
	}
	else {
		return ceil(input);
	}
}

double quadratic(double angle, double speed, double x) {
	return tan(angle) * x - (9.8 / (2 * pow(speed, 2) * pow(cos(angle), 2)) * pow(x, 2));
}

double quadraticDerivative(double angle, double speed, double x) {
	return tan(angle) - 2 * (9.8 / (2 * pow(speed, 2) * pow(cos(angle), 2))) * x;
}

QuadraticRaycastMarcher &QuadraticRaycastMarcher::operator++() {
	if(this->finished()) {
		return *this;
	}

	if(!engine->chunkContainer->isValidTilePosition(this->position)) {
		this->_finished = true;
		this->currentNormal = glm::vec3();
		return *this;
	}

	// update the normals here
	this->updateNormal();

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
					invalidDirectionsForWall[(unsigned char)index][i] != INVALID_DIRECTION
					&& this->neighborDirection < DIRECTION_UP
					&& (
						invalidDirectionsForWall[(unsigned char)index][i] == this->oldNeighborDirection || (
							invalidDirectionsForWall[(unsigned char)index][i] == flipDirection(this->neighborDirection)
							&& !this->canFinish()
						)
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
				invalidDirectionsForWall[(unsigned char)index][i] != INVALID_DIRECTION
				&& this->neighborDirection < DIRECTION_UP
				&& (
					invalidDirectionsForWall[(unsigned char)index][i] == this->oldNeighborDirection || (
						invalidDirectionsForWall[(unsigned char)index][i] == flipDirection(this->neighborDirection)
						&& !this->canFinish()
					)
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

	// march the position
	this->marchPosition();

	return *this;
}

void QuadraticRaycastMarcher::updateNormal() {
	this->oldNeighborDirection = this->neighborDirection;
	
	double distance = glm::distance(glm::vec2(this->start), glm::vec2(position2d));
	double verticalDisplacement = quadratic(this->angle, this->speed, distance);

	if(this->position2d == glm::ivec2(this->end)) {
		if(this->lastZ < this->end.z) {
			this->currentNormal = glm::vec3(0, 0, 1);
			this->neighborDirection = DIRECTION_UP;
		}
		else {
			this->currentNormal = glm::vec3(0, 0, -1);
			this->neighborDirection = DIRECTION_DOWN;
		}
	}
	else if(this->lastZ - verticalDisplacement >= 1) {
		this->currentNormal = glm::vec3(0, 0, -1);
		this->neighborDirection = DIRECTION_DOWN;
	}
	else if(this->lastZ - verticalDisplacement <= -1) {
		this->currentNormal = glm::vec3(0, 0, 1);
		this->neighborDirection = DIRECTION_UP;
	}
	else if(this->bounds.x < this->bounds.y) {
		this->currentNormal = glm::vec3(-this->steps.x, 0, 0);
		
		if(this->steps.x < 0) {
			this->neighborDirection = DIRECTION_WEST;
		}
		else if(this->steps.x > 0) {
			this->neighborDirection = DIRECTION_EAST;
		}
	}
	else {
		this->currentNormal = glm::vec3(0, -this->steps.y, 0);

		if(this->steps.y < 0) {
			this->neighborDirection = DIRECTION_NORTH;
		}
		else if(this->steps.y > 0) {
			this->neighborDirection = DIRECTION_SOUTH;
		}
	}
}

void QuadraticRaycastMarcher::marchPosition() {
	while(this->position2d != glm::ivec2(this->end)) {
		double distance = glm::distance(glm::vec2(this->start), glm::vec2(position2d));
		double verticalDisplacement = quadratic(this->angle, this->speed, distance);

		double add = 1.0;
		bool checkMode = true;
		if(this->lastZ > verticalDisplacement) {
			add = -1.0;
			checkMode = false;
		}

		for(
			double z = this->lastZ;
			(
				(checkMode && z <= verticalDisplacement + 0.00001)
				|| (!checkMode && z >= verticalDisplacement - 0.00001)
			);
			z += add
		) { // project the quadratic from the x/y plane onto the z-axis
			this->position = glm::ivec3(this->position2d.x, this->position2d.y, z + this->start.z);
			if(this->position != this->lastPosition) {
				this->lastPosition = this->position;
				this->lastZ = z;
				return;
			}
		}

		if(this->bounds.x < this->bounds.y) { // do the DDA on the x/y plane
			this->position2d.x += this->steps.x;
			this->bounds.x += this->delta.x;
		}
		else {
			this->position2d.y += this->steps.y;
			this->bounds.y += this->delta.y;
		}
	}

	double add = 1.0;
	bool checkMode = true;
	if(this->lastZ > (double)this->end.z - (double)this->start.z) {
		add = -1.0;
		checkMode = false;
	}

	for(
		double z = this->lastZ;
		(
			(checkMode && z <= (double)this->end.z + 0.00001 - (double)this->start.z)
			|| (!checkMode && z >= (double)this->end.z - 0.00001 - (double)this->start.z)
		);
		z += add
	) {
		this->position = glm::ivec3(this->position2d.x, this->position2d.y, z + this->start.z);

		if(this->position != this->lastPosition) {
			this->lastPosition = this->position;
			this->lastZ = z;
			return;
		}
	}
}

bool QuadraticRaycastMarcher::canFinish() {
	return this->position == this->end;
}

bool QuadraticRaycastMarcher::finished() {
	return this->_finished;
}

std::vector<RaycastResult> quadraticRaycast(glm::ivec3 start, glm::ivec3 end, double speed, unsigned int options) {
	QuadraticRaycastMarcher marcher(start, end, speed, options);
	return marcher.cast();
}

void es::defineQuadraticRaycast() {
	esEntryType projectileAngleArgs[3] = { ES_ENTRY_MATRIX, ES_ENTRY_MATRIX, ES_ENTRY_NUMBER };
	esRegisterFunction(engine->eggscript, ES_ENTRY_OBJECT, es::projectileAngle, "projectileAngle", 3, projectileAngleArgs);

	esEntryType quadraticRaycastArgs[4] = { ES_ENTRY_MATRIX, ES_ENTRY_MATRIX, ES_ENTRY_NUMBER, ES_ENTRY_NUMBER };
	esRegisterFunction(engine->eggscript, ES_ENTRY_OBJECT, es::quadraticRaycast, "quadraticRaycast", 4, quadraticRaycastArgs);
}

esEntryPtr es::projectileAngle(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(
		argc == 3
		&& args[0].matrixData->rows == 3 && args[0].matrixData->columns == 1
		&& args[1].matrixData->rows == 3 && args[1].matrixData->columns == 1
	) {
		glm::vec3 start(
			args[0].matrixData->data[0][0].numberData,
			args[0].matrixData->data[1][0].numberData,
			args[0].matrixData->data[2][0].numberData
		);

		glm::vec3 end(
			args[1].matrixData->data[0][0].numberData,
			args[1].matrixData->data[1][0].numberData,
			args[1].matrixData->data[2][0].numberData
		);

		double speed = args[2].numberData;
		double result = calculateProjectileAngle(start, end, speed);

		if(std::isnan(result)) {
			return nullptr;
		}
		else {
			return esCreateNumber(result);
		}
	}

	return nullptr;
}

esEntryPtr es::quadraticRaycast(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(
		argc >= 3
		&& args[0].matrixData->rows == 3 && args[0].matrixData->columns == 1
		&& args[1].matrixData->rows == 3 && args[1].matrixData->columns == 1
	) {
		std::vector<RaycastResult> result = ::quadraticRaycast(
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
			args[2].numberData,
			(unsigned int)args[3].numberData
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
