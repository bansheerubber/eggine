#include "doubleDimension.h"

template<class T>
T** createDoubleDimensionArray(size_t x, size_t y) {
	T** destination = new T*[x];
	for(int i = 0; i < y; ++i) {
		destination[i] = new T[y];
	}
}
