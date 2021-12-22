#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "../util/dynamicArray.h"

namespace network {
	enum PacketType {
		REMOTE_OBJECT_UPDATE,
	};

	template <class, template <class> class>
	struct is_instance : public std::false_type {};

	template <class T, template <class> class U>
	struct is_instance<U<T>, U> : public std::true_type {};
	
	class Stream {
		friend class Network;
		
		public:			
			void startWriteRemoteObject(unsigned int remoteId);
			void finishWriteRemoteObject(unsigned int remoteId);
			void writeMask(unsigned int size, const char* mask);
			void flush();

			template<class T>
			unsigned int writeNumber(T number) {
				for(int i = sizeof(T) - 1; i >= 0; i--) {
					this->buffer[this->buffer.head] = ((const char*)&number + i)[0]; // dont ask any questions
					this->buffer.pushed();
				}
				return sizeof(T);
			}

			unsigned int writeVector(glm::vec2 vector) {
				this->writeNumber(vector.x);
				this->writeNumber(vector.y);
				return sizeof(vector);
			}

			unsigned int writeVector(glm::vec3 vector) {
				this->writeNumber(vector.x);
				this->writeNumber(vector.y);
				this->writeNumber(vector.z);
				return sizeof(vector);
			}

			unsigned int writeVector(glm::vec4 vector) {
				this->writeNumber(vector.x);
				this->writeNumber(vector.y);
				this->writeNumber(vector.z);
				this->writeNumber(vector.w);
				return sizeof(vector);
			}

			unsigned int writeVector(glm::ivec2 vector) {
				this->writeNumber(vector.x);
				this->writeNumber(vector.y);
				return sizeof(vector);
			}

			unsigned int writeVector(glm::ivec3 vector) {
				this->writeNumber(vector.x);
				this->writeNumber(vector.y);
				this->writeNumber(vector.z);
				return sizeof(vector);
			}

			unsigned int writeVector(glm::ivec4 vector) {
				this->writeNumber(vector.x);
				this->writeNumber(vector.y);
				this->writeNumber(vector.z);
				this->writeNumber(vector.w);
				return sizeof(vector);
			}

			unsigned int writeVector(glm::uvec2 vector) {
				this->writeNumber(vector.x);
				this->writeNumber(vector.y);
				return sizeof(vector);
			}

			unsigned int writeVector(glm::uvec3 vector) {
				this->writeNumber(vector.x);
				this->writeNumber(vector.y);
				this->writeNumber(vector.z);
				return sizeof(vector);
			}

			unsigned int writeVector(glm::uvec4 vector) {
				this->writeNumber(vector.x);
				this->writeNumber(vector.y);
				this->writeNumber(vector.z);
				this->writeNumber(vector.w);
				return sizeof(vector);
			}

			template<class T>
			T readNumber() {
				char reversed[sizeof(T)];
				for(int i = sizeof(T) - 1, j = 0; i >= 0; i--, j++) {
					reversed[j] = this->buffer[this->readBufferPointer + i];
				}

				this->readBufferPointer += sizeof(T);

				return *((T*)(reversed));
			}

			template<class T>
			T readVector();

		private:
			DynamicArray<char> buffer = DynamicArray<char>(4);
			DynamicArray<char> readBuffer = DynamicArray<char>(4);
			unsigned int readBufferPointer = 0;
	};

	template<>
	inline glm::vec2 Stream::readVector<glm::vec2>() {
		return glm::vec2(this->readNumber<float>(), this->readNumber<float>());
	}

	template<>
	inline glm::vec3 Stream::readVector<glm::vec3>() {
		return glm::vec3(this->readNumber<float>(), this->readNumber<float>(), this->readNumber<float>());
	}

	template<>
	inline glm::vec4 Stream::readVector<glm::vec4>() {
		return glm::vec4(this->readNumber<float>(), this->readNumber<float>(), this->readNumber<float>(), this->readNumber<float>());
	}

	template<>
	inline glm::ivec2 Stream::readVector<glm::ivec2>() {
		return glm::ivec2(this->readNumber<int>(), this->readNumber<int>());
	}

	template<>
	inline glm::ivec3 Stream::readVector<glm::ivec3>() {
		return glm::ivec3(this->readNumber<int>(), this->readNumber<int>(), this->readNumber<int>());
	}

	template<>
	inline glm::ivec4 Stream::readVector<glm::ivec4>() {
		return glm::ivec4(this->readNumber<int>(), this->readNumber<int>(), this->readNumber<int>(), this->readNumber<int>());
	}

	template<>
	inline glm::uvec2 Stream::readVector<glm::uvec2>() {
		return glm::uvec2(this->readNumber<unsigned int>(), this->readNumber<unsigned int>());
	}

	template<>
	inline glm::uvec3 Stream::readVector<glm::uvec3>() {
		return glm::uvec3(this->readNumber<unsigned int>(), this->readNumber<unsigned int>(), this->readNumber<unsigned int>());
	}

	template<>
	inline glm::uvec4 Stream::readVector<glm::uvec4>() {
		return glm::uvec4(this->readNumber<unsigned int>(), this->readNumber<unsigned int>(), this->readNumber<unsigned int>(), this->readNumber<unsigned int>());
	}
};
