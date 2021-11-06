#include "tileMath.h"

#include <cmath>
#include <stdio.h>

using namespace std;

long tilemath::coordinateToIndex(glm::ivec2 coordinate, long size) {
	long x = size - coordinate.x - 1, y = coordinate.y;
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

glm::uvec2 tilemath::indexToCoordinate(long index, long size) {
	double threshold = size * (size + 1.0) / 2.0;
	if(index < threshold) {
		double m = floor((sqrt(8.0 * index + 1.0) - 1.0) / 2.0); // elements per row
		double x = index - m * (m + 1.0) / 2.0;
		double y = m - (index - m * (m + 1.0) / 2.0);
		return glm::uvec2(lround(size - x - 1), lround(y));
	}
	else {
		double m = ceil((sqrt(8.0 * -(index - pow(size, 2)) + 1) - 1) / 2.0) - 1;
		double x = index + (m * ((m + 1.0) / 2.0)) - size * size + size;
		double y = -index - (m * (m + 1.0) / 2.0) - m + size * size + size - 2; // lmao how did i ever figure that one out
		return glm::uvec2(lround(size - x - 1), lround(y));
	}
}

glm::vec2 tilemath::tileToScreen(glm::vec3 coordinate) {
	return glm::vec2(
		coordinate.x * 1.0f / 2 + coordinate.y * 1.0f / 2,
		-(coordinate.x * -1.0f / 4 + coordinate.y * 1.0f / 4 - coordinate.z * 39.0 / 64.0)
	);
}

glm::ivec2 tilemath::textureIndexToXY(unsigned int index, unsigned int width, unsigned int height) {
	int spriteWidth = 64.0;
	int spriteHeight = 128.0;
	float padding = 1;
	int spritesOnRow = (int)width / (int)spriteWidth;
	int x = index % spritesOnRow;
	int y = index / spritesOnRow;

	return glm::ivec2(spriteWidth * x + 2 * x + 1, spriteHeight * y + 2 * y + 1);
}

tilemath::TileUV tilemath::textureIndexToUV(unsigned int index, unsigned int width, unsigned int height) {
	float spriteWidth = 64.0;
	float spriteHeight = 128.0;
	float padding = 1;
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
