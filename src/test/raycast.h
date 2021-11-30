#pragma once

#include <eggscript/egg.h>
#include <glm/vec3.hpp>

namespace es {
	void defineRaycast();
	esEntryPtr raycast(esEnginePtr engine, unsigned int argc, esEntryPtr args);
};

struct RaycastResult {
	bool hit;
	glm::ivec3 position;
	glm::vec3 normal;
};

class RaycastMarcher {
	public:
		RaycastMarcher(glm::ivec3 start, glm::vec3 direction, unsigned int length);
		RaycastMarcher(glm::ivec3 start, glm::ivec3 end);

		bool finished();
		glm::ivec3 value(); // intermediate result
		RaycastResult cast();

		RaycastMarcher& operator++();
	
	private:
		glm::ivec3 start;
		glm::ivec3 end;
		unsigned int length;
		bool useEnd = false;
		glm::ivec3 position;
		glm::ivec3 steps;
		glm::vec3 bounds;
		glm::vec3 delta;
		bool _finished = false;
		RaycastResult result;
};

RaycastResult raycast(glm::ivec3 start, glm::vec3 direction, unsigned int length);
RaycastResult raycast(glm::ivec3 start, glm::ivec3 end);
