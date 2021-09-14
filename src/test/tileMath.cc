#include "tileMath.h"

#include <cmath>
#include <stdio.h>

using namespace std;

long tilemath::coordinateToIndex(glm::ivec2 coordinate, long size, bool shouldPrint) {
	long x = coordinate.x, y = coordinate.y;
	if(coordinate.x + y < size) {
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

		if(shouldPrint) {
			printf("%f %f %f %f %f\n", dsize, t, m, q, start);
		}

		return lround(start - q);
	}
}

glm::uvec2 tilemath::indexToCoordinate(unsigned long index, unsigned long size) {
	double dindex = index;
	double dsize = size;
	double threshold = size * (size + 1.0) / 2.0;
	if(index < threshold) {
		double m = floor((sqrt(8.0 * dindex + 1.0) - 1.0) / 2.0); // elements per row
		double x = dindex - m * (m + 1.0) / 2.0;
		double y = m - (dindex - m * (m + 1.0) / 2.0);
		return glm::uvec2(lround(size - x - 1), lround(y));
	}
	else {
		double m = ceil((sqrt(8.0 * -(dindex - pow(size, 2)) + 1) - 1) / 2.0) - 1;
		double x = dindex + (m * ((m + 1.0) / 2.0)) - size * size + size;
		double y = -dindex - (m * (m + 1.0) / 2.0) - m + size * size + size - 2; // lmao how did i ever figure that one out
		return glm::uvec2(lround(size - x - 1), lround(y));
	}
}
