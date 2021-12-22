#pragma once

#include "stream.h"

namespace network {
	class Packet {
		public:
			Packet();
		
			Stream stream = Stream(WRITE);

			void setType(PacketType type);
			PacketType getType();
			unsigned int getSequence();
			void setHeader(unsigned int sequence, unsigned int lastAckedSequence, unsigned long ackMask);
		
		private:
			PacketType type = INVALID_PACKET;
			unsigned int sequence = 0;
	};
};
