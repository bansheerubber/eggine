#include "cloneString.h"
#include <cstring>
#include <cstdint>

using namespace std;

char* cloneString(const char* input) {
	const uint64_t size = strlen(input);
	char* output = new char[size + 1];
	strncpy(output, input, size + 1);
	output[size] = '\0';
	return output;
}
