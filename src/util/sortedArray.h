#pragma once

#include "dynamicArray.h"

template <typename T, typename S = void>
class SortedArray {
	public:
		SortedArray() {

		}

		SortedArray(
			int (*compare) (T* a, T* b),
			void (*init) (S* parent, T* location) = nullptr,
			void (*onRealloc) (S* parent) = nullptr
		) {
			this->compare = compare;
			new((void*)&this->array) DynamicArray<T, S>(4, init, onRealloc);
		}
		
		SortedArray(
			S* parent,
			int (*compare) (T* a, T* b),
			void (*init) (S* parent, T* location) = nullptr,
			void (*onRealloc) (S* parent) = nullptr
		) {
			this->compare = compare;
			new((void*)&this->array) DynamicArray<T, S>(parent, 4, init, onRealloc);
		}

		void insert(T value) {
			// find a place to insert it at
			for(unsigned long i = 0; i < this->array.head; i++) {
				if(this->compare(&value, &this->array[i]) < 0) {
					this->array.shift(i, 1);
					this->array[i] = value;
					return;
				}
			}
			
			// insert at the end
			size_t iterator = this->array.head;
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
			qsort(&this->array[0], this->array.head, sizeof(T), (int (*)(const void*, const void*))this->compare);
		}
	
		DynamicArray<T, S> array;
	
	private:
		int (*compare) (T* a, T* b);

		void swap(size_t index1, size_t index2) {
			T temp = this->array[index1];
			this->array[index1] = this->array[index2];
			this->array[index2] = temp;
		}
};
