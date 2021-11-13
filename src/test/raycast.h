#pragma once

#include <glm/vec3.hpp>

struct RaycastResult {
	bool hit;
	glm::ivec3 position;
	glm::vec3 normal;
};

class RaycastMarcher {
	public:
		RaycastMarcher(glm::ivec3 start, glm::vec3 direction, unsigned int length);

		bool finished();
		glm::ivec3 value(); // intermediate result
		RaycastResult cast();

		RaycastMarcher& operator++();
	
	private:
		glm::ivec3 position;
		glm::ivec3 steps;
		glm::vec3 bounds;
		glm::vec3 delta;
		bool _finished = false;
		RaycastResult result;
};

RaycastResult raycast(glm::ivec3 start, glm::vec3 direction, unsigned int length);
