#include "time.h"

#include <chrono>

uint64_t getMicrosecondsNow() {
	return std::chrono::duration_cast<std::chrono::microseconds>(
		std::chrono::high_resolution_clock::now().time_since_epoch()
	).count();
}
