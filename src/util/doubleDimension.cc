#include "doubleDimension.h"

template<class T>
T** createDoubleDimensionArray(uint64_t x, uint64_t y) {
	T** destination = new T*[x];
	for(int i = 0; i < y; ++i) {
		destination[i] = new T[y];
	}
}
