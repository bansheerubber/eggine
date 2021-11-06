#pragma once

inline char lsb(unsigned int number) {
	char count = 0;
	unsigned int currentNumber = number;
	for(unsigned int i = 0; i < 32; i++) {
		if(currentNumber & 0b1) {
			return count;
		}
		currentNumber >>= 1;
		count++;
	}
	return -1;
}
