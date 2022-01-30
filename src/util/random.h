#pragma once

#include <random>

std::default_random_engine generator;

uint64_t randomLong() {
	std::uniform_int_distribution<uint64_t> distribution(0, 0xFFFFFFFFFFFFFFFF);
	return distribution(generator);
}

unsigned int randomInt() {
	std::uniform_int_distribution<unsigned int> distribution(0, 0xFFFFFFFF);
	return distribution(generator);
}
