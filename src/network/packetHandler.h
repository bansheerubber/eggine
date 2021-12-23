#pragma once

#include "stream.h"

namespace network {
	struct PacketNode {
		class Packet* packet;
		PacketNode* next;
		PacketNode* previous;

		PacketNode(Packet* packet) {
			this->packet = packet;
			this->next = nullptr;
			this->previous = nullptr;
		}
	};

	class PacketHandler {
		public:
			void readPacket();
			virtual void sendPacket(class Packet* packet);
			virtual void ackPacket(unsigned int sequence);
			virtual void resendPacket(unsigned int sequence);

		protected:
			Stream* receiveStream = new Stream();

			unsigned int lastSequenceReceived = 0;
			unsigned long lastHighestAckReceived;
			unsigned long ackMask = 0; // mask for all packets that we've acknowledged receiving

			unsigned int lastSequenceSent = 0;

			PacketNode* head = nullptr;

			unsigned int packetListSize();
			void deleteNodeFromList(unsigned int sequence);
			void deleteNodeAndPacketFromList(unsigned int sequence);
	};
};
