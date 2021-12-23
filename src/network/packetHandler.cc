#include "packetHandler.h"

#include "packet.h"

void network::PacketHandler::sendPacket(class Packet* packet) {
	if(this->head == nullptr) {
		this->head = new PacketNode(packet);
	}	
	else {
		PacketNode* node = this->head;
		this->head = new PacketNode(packet);
		this->head->next = node;
		node->previous = this->head;
	}
}

void network::PacketHandler::ackPacket(unsigned int sequence) {
	this->deleteNodeAndPacketFromList(sequence);
}

void network::PacketHandler::resendPacket(unsigned int sequence) {
	this->deleteNodeFromList(sequence);
}

void network::PacketHandler::deleteNodeFromList(unsigned int sequence) {
	PacketNode* node = this->head;
	while(node != nullptr) {
		if(node->packet->getSequence() == sequence) {
			break;
		}
		node = node->next;
	}

	if(node == nullptr) {
		return;
	}

	if(node == this->head) {
		if(node->next != nullptr) {
			this->head = node->next;
		}
		else {
			this->head = nullptr;
		}

		delete node;
	}
	else {
		if(node->next != nullptr) {
			node->previous->next = node->next;
			node->next->previous = node->previous;
		}
		else {
			node->previous->next = nullptr;
		}

		delete node;
	}
}

void network::PacketHandler::deleteNodeAndPacketFromList(unsigned int sequence) {
	PacketNode* node = this->head;
	while(node != nullptr) {
		if(node->packet->getSequence() == sequence) {
			break;
		}
		node = node->next;
	}

	if(node == nullptr) {
		return;
	}

	if(node == this->head) {
		if(node->next != nullptr) {
			this->head = node->next;
		}
		else {
			this->head = nullptr;
		}

		delete node->packet;
		delete node;
	}
	else {
		if(node->next != nullptr) {
			node->previous->next = node->next;
			node->next->previous = node->previous;
		}
		else {
			node->previous->next = nullptr;
		}

		delete node->packet;
		delete node;
	}
}

void network::PacketHandler::readPacket() {
	PacketType type = (PacketType)this->receiveStream.readNumber<char>();
	unsigned int receivedSequence = this->receiveStream.readNumber<unsigned int>();
	unsigned int receivedHighestSequenceAck = this->receiveStream.readNumber<unsigned int>();
	
	// ack mask is a mask of the messages the client has successfully received, starting at highestSequenceAck and then heading downwards
	unsigned long receivedAckMask = this->receiveStream.readNumber<unsigned long>();

	if(type == INVALID_PACKET || type >= MAX_PACKET) {
		return;
	}

	this->ackMask <<= receivedSequence - this->lastSequenceReceived; // update our mask
	this->ackMask |= 1;

	// printf("-----------------------------------------------\n");
	// printf("packet check:\n");
	// printf("packets in list: %u\n", this->packetListSize());
	// printf("their last ack: %d\n", receivedHighestSequenceAck);
	// printf("our remembered last ack: %d\n", lastHighestAckReceived);
	// printf("their ack mask: ");
	// for(unsigned long i = 0; i < 64; i++) {
	// 	printf("%d", (receivedAckMask & ((unsigned long)1 << i)) > 0);
	// }
	// printf("\n");

	if(lastHighestAckReceived == 0) {
		lastHighestAckReceived = receivedHighestSequenceAck - 1;
	}

	// do ack check. if we think that the connection didn't get packets that we sent, then we're going to re-send them
	if(this->head != nullptr) {
		for(unsigned int i = lastHighestAckReceived + 1; i <= receivedHighestSequenceAck; i++) {
			unsigned int checkBit = receivedHighestSequenceAck - i;
			if(checkBit >= 64) {
				// printf("dropped packet from outside the ack window #%d\n", i);
				this->resendPacket(i);
			}
			else if(receivedAckMask & (1 << checkBit)) {
				// printf("acked packet #%d\n", i);
				this->ackPacket(i);
			}
			else {
				// printf("dropped packet #%d\n", i);
				this->resendPacket(i);
			}
		}
	}

	// printf("packets left in list: %u\n", this->packetListSize());

	this->lastHighestAckReceived = receivedHighestSequenceAck;
	this->lastSequenceReceived = receivedSequence;
	// printf("-----------------------------------------------\n");
	// printf("packet recieved: %u\n", this->lastSequenceReceived);
	// printf("our ack mask: ");
	// for(unsigned long i = 0; i < 64; i++) {
	// 	printf("%d", (this->ackMask & ((unsigned long)1 << i)) > 0);
	// }
	// printf("\n");
}

unsigned int network::PacketHandler::packetListSize() {
	PacketNode* node = this->head;
	unsigned int count = 0;
	while(node != nullptr) {
		node = node->next;
		count++;
	}
	return count;
}
