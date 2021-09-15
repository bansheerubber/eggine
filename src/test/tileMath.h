#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace tilemath {
	// convert an x y coordinate into a opengl buffer index
	long coordinateToIndex(glm::ivec2 coordinate, long size, bool shouldPrint = false);

	// convert an opengl buffer index into x y coordinates
	glm::uvec2 indexToCoordinate(long index, long size);

	// convert tile coordinates to screenspace
	glm::vec2 tileToScreen(glm::vec3 coordinate);
}
