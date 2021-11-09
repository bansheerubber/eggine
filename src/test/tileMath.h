#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace tilemath {
	enum Rotation {
		ROTATION_0_DEG,
		ROTATION_90_DEG,
		ROTATION_180_DEG,
		ROTATION_270_DEG,
	};
	
	// convert an x y coordinate into a opengl buffer index
	long coordinateToIndex(glm::ivec2 coordinate, long size, Rotation rotation);
	// convert an opengl buffer index into x y coordinates
	glm::uvec2 indexToCoordinate(long index, long size, Rotation rotation);

	// convert tile coordinates to screenspace
	glm::vec2 tileToScreen(glm::vec3 coordinate, Rotation rotation);

	struct TileUV {
		glm::vec2 minimum;
		glm::vec2 maximum;
	};

	glm::ivec2 textureIndexToXY(unsigned int index, unsigned int width, unsigned int height);
	TileUV textureIndexToUV(unsigned int index, unsigned int width, unsigned int height);
}
