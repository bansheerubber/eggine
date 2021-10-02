#pragma once

#include "dynamicArray.h"

template <typename T, typename S>
class SortedArray {
	public:
		SortedArray() {

		}
		
		SortedArray(
			S* parent,
			int (*compare) (const void* a, const void* b),
			void (*init) (S* parent, T* location),
			void (*onRealloc) (S* parent)
		) {
			this->compare = compare;
			new((void*)&this->array) DynamicArray<T, S>(parent, 4, init, onRealloc);
		}

		void insert(T value) {
			// insert at the end
			size_t iterator = this->array.head;
			this->array[iterator] = value;
			this->array.pushed();

			this->sort();
		}

		void remove(T value) {
			// find the value
			size_t found = -1;
			for(size_t i = 0; i < this->array.head; i++) {
				if(this->array[i] == value) {
					found = i;
					break;
				}
			}

			if(found != -1) {
				this->swap(found, this->array.head - 1);
				this->array.popped();
				this->sort();
			}
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
			qsort(&this->array[0], this->array.head, sizeof(T), this->compare);
		}
	
		DynamicArray<T, S> array;
	
	private:
		int (*compare) (const void* a, const void* b);

		void swap(size_t index1, size_t index2) {
			T temp = this->array[index1];
			this->array[index1] = this->array[index2];
			this->array[index2] = temp;
		}
};
