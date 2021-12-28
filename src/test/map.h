#pragma once

#include <cstdint>
#include <fstream>
#include <tsl/robin_map.h>
#include <string.h>
#include <type_traits>

using namespace std;

enum MapCommand {
	MAP_INFO = 0,
	MAP_CHUNK = 1,
	MAP_SIZE = 2,
};

class Map {
	public:
		Map(class ChunkContainer* container);
		void loadFromFile(string filename);
		void load(unsigned char* buffer, uint64_t size);
		void save(string filename);
	
	private:
		class ChunkContainer* container;

		tsl::robin_map<string, string> info;

		static constexpr uint64_t Version = 1;
		
		template<class T>
		T readNumber(unsigned char* buffer, uint64_t* index) {
			*index += sizeof(T);
			
			if constexpr(is_same<float, T>::value || is_same<double, T>::value) {
				uint32_t temp1 = 0;
				uint64_t temp2 = 0;
				for(uint64_t i = 0, shift = (sizeof(T) - 1) * 8; i < sizeof(T); i++, shift -= 8) {
					if constexpr(is_same<float, T>::value) {
						temp1 |= (uint64_t)buffer[i] << shift;
					}
					else {
						temp2 |= (uint64_t)buffer[i] << shift;
					}
				}

				T output;
				if constexpr(is_same<float, T>::value) {
					memcpy(&output, &temp1, sizeof(T));
				}
				else {
					memcpy(&output, &temp2, sizeof(T));
				}
				return output;
			}
			else {
				T output = 0;
				for(uint64_t i = 0, shift = (sizeof(T) - 1) * 8; i < sizeof(T); i++, shift -= 8) {
					output |= (uint64_t)buffer[i] << shift;
				}
				return output;
			}
		}

		string readString(unsigned char* buffer, uint64_t* index) {
			uint16_t size = this->readNumber<uint16_t>(buffer, index);
			char output[size];
			for(unsigned int i = 0; i < size; i++) {
				output[i] = buffer[sizeof(uint16_t) + i];
				(*index)++;
			}
			return string(output, size);
		}

		template<class T>
		void writeNumber(ofstream &file, T number) {
			if constexpr(is_same<float, T>::value || is_same<double, T>::value) {
				uint32_t temp1 = 0;
				uint64_t temp2 = 0;
				if constexpr(is_same<float, T>::value) {
					memcpy(&temp1, &number, sizeof(T));
				}
				else {
					memcpy(&temp2, &number, sizeof(T));
				}

				for(uint64_t i = 0, shift = (sizeof(T) - 1) * 8; i < sizeof(T); i++, shift -= 8) {
					if constexpr(is_same<float, T>::value) {
						file.put((temp1 >> shift) & 0xFF);
					}
					else {
						file.put((temp2 >> shift) & 0xFF);
					}
				}
			}
			else {
				for(uint64_t i = 0, shift = (sizeof(T) - 1) * 8; i < sizeof(T); i++, shift -= 8) {
					file.put((number >> shift) & 0xFF);
				}
			}
		}

		void writeString(ofstream &file, string input) {
			this->writeNumber(file, (uint16_t)input.size());
			file.write(input.c_str(), (uint16_t)input.size());
		}
};
