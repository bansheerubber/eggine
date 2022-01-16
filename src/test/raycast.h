#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_GTX_hash

#include <eggscript/egg.h>
#include <glm/gtx/hash.hpp>
#include <tsl/robin_set.h>
#include <glm/vec3.hpp>
#include <vector>

#include "neighbors.h"

namespace es {
	void defineRaycast();
	esEntryPtr raycast(esEnginePtr engine, unsigned int argc, esEntryPtr args);
};

enum RaycastOptions {
	RAYCAST_PENETRATE = 0b00000001,
};

struct RaycastResult {
	glm::ivec3 position;
	glm::vec3 normal;
};

class RaycastMarcher {
	public:
		RaycastMarcher(glm::ivec3 start, glm::vec3 direction, unsigned int length, unsigned int options);
		RaycastMarcher(glm::ivec3 start, glm::ivec3 end, unsigned int options);

		bool finished();
		glm::ivec3 value(); // intermediate result
		std::vector<RaycastResult>& cast();

		RaycastMarcher& operator++();
	
	private:
		glm::ivec3 start;
		glm::ivec3 end;
		unsigned int options = 0;
		unsigned int length;
		bool useEnd = false;
		NeighborDirection direction = INVALID_DIRECTION;
		glm::ivec3 position;
		glm::ivec3 steps;
		glm::vec3 bounds;
		glm::vec3 delta;
		bool _finished = false;
		std::vector<RaycastResult> results;
		tsl::robin_set<glm::ivec3> currentHits;
		glm::vec3 currentNormal;
};

std::vector<RaycastResult> raycast(glm::ivec3 start, glm::vec3 direction, unsigned int length, unsigned int options);
std::vector<RaycastResult> raycast(glm::ivec3 start, glm::ivec3 end, unsigned int options);
