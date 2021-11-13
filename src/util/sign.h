#pragma once

template <typename T>
inline int sign(T val) {
	return (T(0) < val) - (val < T(0));
}
