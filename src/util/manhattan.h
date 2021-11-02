#pragma once

#include <cmath>
#include <glm/vec3.hpp>

namespace glm {
	inline int manhattan(glm::ivec3 &a, glm::ivec3 &b) {
		return abs(a.x - b.x) + abs(a.y - b.y) + abs(a.z - b.z);
	}
};
