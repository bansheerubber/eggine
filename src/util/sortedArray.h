#pragma once

#include <algorithm>

#include "dynamicArray.h"

template <typename T>
class SortedArray {
	public:
		SortedArray() {
			new((void*)&this->array) DynamicArray<T>(4);
		}
		void insert(T value) {
			// find a place to insert it at
			for(uint64_t i = 0; i < this->array.head; i++) {
				if(value < this->array[i]) {
					this->array.shift(i, 1);
					this->array[i] = value;
					return;
				}
			}
			
			// insert at the end
			uint64_t iterator = this->array.head;
			this->array[iterator] = value;
			this->array.pushed();
		}

		void remove(T value) {
			this->array.remove(value);
		}

		T& top() {
			return this->array[0];
		}

		void pop() {
			if(this->array.head == 1) {
				this->array.popped();
			}
			else {
				this->array[0] = this->array[this->array.head - 1];
				this->array.popped();

				this->sort();
			}
		}

		void sort() {
			std::sort(
				&this->array[0],
				&this->array[0] + this->array.head,
				[](const T &a, const T &b){ return a < b; }
			);
		}
	
		DynamicArray<T> array;
	
	private:
		void swap(uint64_t index1, uint64_t index2) {
			T temp = this->array[index1];
			this->array[index1] = this->array[index2];
			this->array[index2] = temp;
		}
};
