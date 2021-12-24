#pragma once

#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <string>

namespace network {
	#define EGGINE_PACKET_SIZE 32768
	
	typedef unsigned short remote_class_id;
	typedef unsigned long remote_object_id;

	enum StreamType {
		REMOTE_OBJECT_CREATE = 1,
		REMOTE_OBJECT_UPDATE,
	};

	enum PacketType {
		INVALID_PACKET,
		DROPPABLE_PACKET,
		MAX_PACKET,
	};

	// largest update mask possible (240 properties)
	struct UpdateMask {
		unsigned char mask[32];
		unsigned char size;

		UpdateMask() {
			for(unsigned int i = 0; i < 32; i++) {
				this->mask[i] = 0;
			}
		}

		bool read(unsigned int bit) const {
			unsigned int charIndex = bit / 8;
			if(charIndex >= this->size) {
				return false;
			}

			unsigned int maskIndex = bit % 8;
			return this->mask[charIndex] & (1 << maskIndex);
		}

		std::string toString() const {
			std::string result;
			for(unsigned char i = 0; i < this->size * sizeof(char) * 8; i++) {
				result += std::to_string(this->read(i));
			}
			return result;
		}
	};

	#define EGGINE_NETWORK_MAX_UPDATE_MASK_SIZE 32 // anything higher is an invalid mask

	class RemoteObjectUnpackException: public std::exception {
		public:
			RemoteObjectUnpackException(class RemoteObject* object, std::string note) {
				this->object = object;
				this->note = note;
			}

			class RemoteObject* object;
			std::string note;

			virtual const char* what() const throw() {
				return this->note.c_str();
			}
	};

	class RemoteObjectIdMisMatchException: public std::exception {
		public:
			RemoteObjectIdMisMatchException(remote_object_id expectedId, remote_object_id receivedId) {
				this->expectedId = expectedId;
				this->receivedId = receivedId;
				this->message = fmt::format("Expected remote object id {}, got {}", expectedId, receivedId);
			}

			remote_object_id expectedId;
			remote_object_id receivedId;

			virtual const char* what() const throw() {
				return message.c_str();
			}
		
		protected:
			std::string message;
	};

	class StreamOverReadException: public std::exception {
		public:
			StreamOverReadException(size_t size, unsigned int attemptedRead) {
				this->size = size;
				this->attemptedRead = attemptedRead;
				this->message = fmt::format("Over-read trying to read {} bytes from stream of size {}", attemptedRead, size);
			}

			size_t size;
			unsigned int attemptedRead;

			virtual const char* what() const throw() {
				return message.c_str();
			}
		
		protected:
			std::string message;
	};

	class RemoteObjectInstantiateException: public std::exception {
		public:
			RemoteObjectInstantiateException(remote_object_id objectId, remote_class_id classId) {
				this->classId = classId;
				this->objectId = objectId;
				this->message = fmt::format("Could not instantiate remote object {} with class {}", objectId, classId);
			}

			remote_class_id classId;
			remote_object_id objectId;

			virtual const char* what() const throw() {
				return message.c_str();
			}
		
		protected:
			std::string message;
	};

	class RemoteObjectLookupException: public std::exception {
		public:
			RemoteObjectLookupException(remote_object_id objectId) {
				this->objectId = objectId;
				this->message = fmt::format("Could not lookup remote object {}", objectId);
			}

			remote_object_id objectId;

			virtual const char* what() const throw() {
				return message.c_str();
			}
		
		protected:
			std::string message;
	};
};