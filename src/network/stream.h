#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "../util/dynamicArray.h"
#include "types.h"

namespace network {
	enum StreamFlags {
		READ = 1,
		WRITE = 2,
		NO_MASK_CHECKING = 4,
	};

	template <class, template <class> class>
	struct is_instance : public std::false_type {};

	template <class T, template <class> class U>
	struct is_instance<U<T>, U> : public std::true_type {};
	
	class Stream {
		friend class Network;
		friend class Connection;
		friend class Client;
		friend class Packet;
		
		public:			
			Stream(unsigned int flags = READ);
			
			void setFlags(unsigned int flags);
			void allocate(size_t size);
			void startChunk();
			void commitChunk();
			void startWriteRemoteObject(class RemoteObject* object);
			void finishWriteRemoteObject(class RemoteObject* object);
			void finishReadRemoteObject(class RemoteObject* object);
			void writeUpdateMask(unsigned int size, const unsigned char* mask);
			const UpdateMask readUpdateMask();
			bool queryMask(RemoteObject* object, unsigned int position);

			bool canRead(unsigned int size) {
				return this->readBufferPointer + size <= this->buffer.head;
			}

			void flush();
			size_t size();
			const char* start();

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
				if(!this->canRead(sizeof(T))) {
					throw 0;
				}
				
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
			unsigned int readBufferPointer = 0;
			unsigned int flags;
			size_t chunkHead = 0;			
	};

	template<>
	inline glm::vec2 Stream::readVector<glm::vec2>() {
		float x = this->readNumber<float>();
		float y = this->readNumber<float>();
		return glm::vec2(x, y);
	}

	template<>
	inline glm::vec3 Stream::readVector<glm::vec3>() {
		float x = this->readNumber<float>();
		float y = this->readNumber<float>();
		float z = this->readNumber<float>();
		return glm::vec3(x, y, z);
	}

	template<>
	inline glm::vec4 Stream::readVector<glm::vec4>() {
		float x = this->readNumber<float>();
		float y = this->readNumber<float>();
		float z = this->readNumber<float>();
		float w = this->readNumber<float>();
		return glm::vec4(x, y, z, w);
	}

	template<>
	inline glm::ivec2 Stream::readVector<glm::ivec2>() {
		int x = this->readNumber<int>();
		int y = this->readNumber<int>();
		return glm::ivec2(x, y);
	}

	template<>
	inline glm::ivec3 Stream::readVector<glm::ivec3>() {
		int x = this->readNumber<int>();
		int y = this->readNumber<int>();
		int z = this->readNumber<int>();
		return glm::ivec3(x, y, z);
	}

	template<>
	inline glm::ivec4 Stream::readVector<glm::ivec4>() {
		int x = this->readNumber<int>();
		int y = this->readNumber<int>();
		int z = this->readNumber<int>();
		int w = this->readNumber<int>();
		return glm::ivec4(x, y, z, w);
	}

	template<>
	inline glm::uvec2 Stream::readVector<glm::uvec2>() {
		unsigned int x = this->readNumber<unsigned int>();
		unsigned int y = this->readNumber<unsigned int>();
		return glm::uvec2(x, y);
	}

	template<>
	inline glm::uvec3 Stream::readVector<glm::uvec3>() {
		unsigned int x = this->readNumber<unsigned int>();
		unsigned int y = this->readNumber<unsigned int>();
		unsigned int z = this->readNumber<unsigned int>();
		return glm::uvec3(x, y, z);
	}

	template<>
	inline glm::uvec4 Stream::readVector<glm::uvec4>() {
		unsigned int x = this->readNumber<unsigned int>();
		unsigned int y = this->readNumber<unsigned int>();
		unsigned int z = this->readNumber<unsigned int>();
		unsigned int w = this->readNumber<unsigned int>();
		return glm::uvec4(x, y, z, w);
	}
};
