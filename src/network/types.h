#pragma once

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
};