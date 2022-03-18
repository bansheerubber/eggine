#include "tileMath.h"

#include <cmath>
#include <stdio.h>

int64_t tilemath::coordinateToIndex(glm::ivec2 coordinate, int64_t size, tilemath::Rotation rotation) {
	int64_t x = 0, y = 0;
	switch(rotation) {
		case ROTATION_0_DEG: {
			x = size - coordinate.x - 1;
			y = coordinate.y;
			break;
		}

		case ROTATION_90_DEG: {
			x = size - coordinate.y - 1;
			y = size - coordinate.x - 1;
			break;
		}

		case ROTATION_180_DEG: {
			x = coordinate.x;
			y = size - coordinate.y - 1;
			break;
		}

		case ROTATION_270_DEG: {
			x = coordinate.y;
			y = coordinate.x;
			break;
		}
	}


	if(x + y < size) {
		double q = x;
		double m = y + q;
		return lround(q + m * (m + 1) / 2);
	}
	else {
		double dsize = size;
		double t = x + y;
		double m = t - (dsize - 1);
		double q = dsize - x - 1;
		double start = -((m - dsize) * ((m - size + 1.0) / 2.0)) + size * size - 1.0;
		return lround(start - q);
	}
}

glm::uvec2 tilemath::indexToCoordinate(int64_t index, int64_t size, tilemath::Rotation rotation) {
	double threshold = size * (size + 1.0) / 2.0;
	double x = 0.0, y = 0.0;
	if(index < threshold) {
		double m = floor((sqrt(8.0 * index + 1.0) - 1.0) / 2.0); // elements per row
		x = index - m * (m + 1.0) / 2.0;
		y = m - (index - m * (m + 1.0) / 2.0);
	}
	else {
		double m = ceil((sqrt(8.0 * -(index - pow(size, 2)) + 1) - 1) / 2.0) - 1;
		x = index + (m * ((m + 1.0) / 2.0)) - size * size + size;
		y = -index - (m * (m + 1.0) / 2.0) - m + size * size + size - 2; // lmao how did i ever figure that one out
	}

	switch(rotation) {
		default:
		case ROTATION_0_DEG: {
			return glm::uvec2(
				lround(size - x - 1),
				lround(y)
			);
		}

		case ROTATION_90_DEG: {
			return glm::uvec2(
				lround(size - y - 1),
				lround(size - x - 1)
			);
		}

		case ROTATION_180_DEG: {
			return glm::uvec2(
				lround(x),
				lround(size - y - 1)
			);
		}

		case ROTATION_270_DEG: {
			return glm::uvec2(
				lround(y),
				lround(x)
			);
		}
	}
}

glm::vec3 tilemath::tileToScreen(glm::vec3 coordinate, int64_t size, tilemath::Rotation rotation) {
	int depth = 0;
	if(size != 0) {
		uint64_t index = tilemath::coordinateToIndex(coordinate, size, rotation);
		double threshold = size * (size + 1.0) / 2.0;
		int row;
		if(index < threshold) {
			row = floor((sqrt(8.0 * index + 1.0) - 1.0) / 2.0);
		}
		else {
			row = (size - 1) - (ceil((sqrt(8.0 * -(index - pow(size, 2)) + 1) - 1) / 2.0) - 1) + (size - 1);
		}
		depth = (row - 1) + coordinate.z * size * size;
	}
	
	switch(rotation) {
		default:
		case ROTATION_0_DEG: {
			return glm::vec3(
				coordinate.x * 1.0f / 2 + coordinate.y * 1.0f / 2,
				-(coordinate.x * -1.0f / 4 + coordinate.y * 1.0f / 4 - coordinate.z * 39.0 / 64.0),
				depth
			);
		}

		case ROTATION_90_DEG: {
			return glm::vec3(
				-(coordinate.x * 1.0f / 2 + coordinate.y * -1.0f / 2),
				-(coordinate.x * -1.0f / 4 + coordinate.y * -1.0f / 4 - coordinate.z * 39.0 / 64.0),
				depth
			);
		}

		case ROTATION_180_DEG: {
			return glm::vec3(
				coordinate.x * -1.0f / 2 + coordinate.y * -1.0f / 2,
				-(coordinate.x * 1.0f / 4 + coordinate.y * -1.0f / 4 - coordinate.z * 39.0 / 64.0),
				depth
			);
		}

		case ROTATION_270_DEG: {
			return glm::vec3(
				-(coordinate.x * -1.0f / 2 + coordinate.y * 1.0f / 2),
				-(coordinate.x * 1.0f / 4 + coordinate.y * 1.0f / 4 - coordinate.z * 39.0 / 64.0),
				depth
			);
		}
	}
}

glm::ivec2 tilemath::textureIndexToXY(unsigned int index, unsigned int width, unsigned int height) {
	int spriteWidth = 64.0;
	int spriteHeight = 128.0;
	// float padding = 1;
	int spritesOnRow = (int)width / (int)spriteWidth;
	int x = index % spritesOnRow;
	int y = index / spritesOnRow;

	return glm::ivec2(spriteWidth * x + 2 * x + 1, spriteHeight * y + 2 * y + 1);
}

tilemath::TileUV tilemath::textureIndexToUV(unsigned int index, unsigned int width, unsigned int height) {
	float spriteWidth = 64.0;
	float spriteHeight = 128.0;
	// float padding = 1;
	int spritesOnRow = (int)width / (int)spriteWidth;
	int x = index % spritesOnRow;
	int y = index / spritesOnRow;

	return {
		minimum: glm::vec2(
			(spriteWidth * float(x) + 2.0 * float(x) + 1.0) / width,
			(spriteHeight * float(y) + 2.0 * float(y) + 1.0) / height
		),
		maximum: glm::vec2(
			(spriteWidth * float(x) + 2.0 * float(x) + 1.0 + spriteWidth) / width,
			(spriteHeight * float(y) + 2.0 * float(y) + 1.0 + spriteHeight) / height
		),
	};
}

glm::ivec2 tilemath::directionTowardsCamera(Rotation rotation) {
	switch(rotation) {
		case tilemath::ROTATION_0_DEG: {
			return glm::ivec2(-1, 1);
		}

		case tilemath::ROTATION_90_DEG: {
			return glm::ivec2(-1, -1);
		}

		case tilemath::ROTATION_180_DEG: {
			return glm::ivec2(1, -1);
		}

		case tilemath::ROTATION_270_DEG: {
			return glm::ivec2(1, 1);
		}
	}

	return glm::ivec2(0, 0);
}
