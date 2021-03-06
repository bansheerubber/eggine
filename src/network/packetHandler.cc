#include "packetHandler.h"

#include "../engine/console.h"
#include "packet.h"

network::PacketHandler::~PacketHandler() {
	delete this->receiveStream;	
}

void network::PacketHandler::readPacket() {
	PacketType type = (PacketType)this->receiveStream->readNumber<char>();
	unsigned int receivedSequence = this->receiveStream->readNumber<unsigned int>();
	unsigned int receivedHighestSequenceAck = this->receiveStream->readNumber<unsigned int>();
	
	// ack mask is a mask of the messages the client has successfully received, starting at highestSequenceAck and then heading downwards
	uint64_t receivedAckMask = this->receiveStream->readNumber<uint64_t>();

	if(type == INVALID_PACKET || type >= MAX_PACKET) {
		return;
	}

	this->ackMask <<= receivedSequence - this->lastSequenceReceived; // update our mask
	this->ackMask |= 1;

	console::print("-----------------------------------------------\n");
	console::print("packet check:\n");
	console::print("packets in list: %u\n", this->packetListSize());
	console::print("their last ack: %u\n", receivedHighestSequenceAck);
	console::print("our remembered last ack: %u\n", lastHighestAckReceived);
	console::print("their ack mask: ");
	for(uint64_t i = 0; i < 64; i++) {
		console::print("%d", (receivedAckMask & ((uint64_t)1 << i)) > 0);
	}
	console::print("\n");

	if(lastHighestAckReceived == 0) {
		lastHighestAckReceived = receivedHighestSequenceAck - 1;
	}

	// do ack check. if we think that the connection didn't get packets that we sent, then we're going to re-send them
	for(unsigned int i = lastHighestAckReceived + 1; i <= receivedHighestSequenceAck; i++) {
		unsigned int checkBit = receivedHighestSequenceAck - i;
		if(checkBit >= 64) {
			console::print("dropped packet from outside the ack window #%u\n", i);
			this->resendPacket(i);
		}
		else if(receivedAckMask & (1 << checkBit)) {
			console::print("acked packet #%u\n", i);
			this->ackPacket(i);
		}
		else {
			console::print("dropped packet #%u\n", i);
			this->resendPacket(i);
		}
	}

	console::print("packets left in list: %u\n", this->packetListSize());

	this->lastHighestAckReceived = receivedHighestSequenceAck;
	this->lastSequenceReceived = receivedSequence;
	console::print("-----------------------------------------------\n");
	console::print("packet recieved: %u %ld\n", this->lastSequenceReceived, this->receiveStream->size());
	console::print("our ack mask: ");
	for(uint64_t i = 0; i < 64; i++) {
		console::print("%d", (this->ackMask & ((uint64_t)1 << i)) > 0);
	}
	console::print("\n");

	this->handlePacket();
}

void network::PacketHandler::sendPacket(class Packet* packet) {
	if(this->head == nullptr) {
		this->head = new PacketNode(packet);
	}	
	else {
		PacketNode* node = this->head;
		this->head = new PacketNode(packet);
		this->head->next = node;
	}
}

void network::PacketHandler::ackPacket(unsigned int sequence) {
	this->deleteNodeAndPacketFromList(sequence);
}

void network::PacketHandler::resendPacket(unsigned int sequence) {
	this->deleteNodeFromList(sequence);
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

void network::PacketHandler::deleteNodeFromList(unsigned int sequence) {
	if(this->head == nullptr) {
		return;
	}

	if(this->head->packet->getSequence() == sequence) {
		PacketNode* deletedNode = this->head;
		this->head = this->head->next;
		delete deletedNode;
	}
	
	PacketNode* node = this->head;
	while(node != nullptr) {
		if(node->next != nullptr && node->next->packet->getSequence() == sequence) {
			break;
		}
		node = node->next;
	}

	if(node == nullptr) {
		return;
	}

	// delete the next node
	PacketNode* deletedNode = node->next;
	node->next = deletedNode->next;
	delete deletedNode;
}

void network::PacketHandler::deleteNodeAndPacketFromList(unsigned int sequence) {
	if(this->head == nullptr) {
		return;
	}

	if(this->head->packet->getSequence() == sequence) {
		PacketNode* deletedNode = this->head;
		this->head = this->head->next;
		delete deletedNode->packet;
		delete deletedNode;
	}
	
	PacketNode* node = this->head;
	while(node != nullptr) {
		if(node->next != nullptr && node->next->packet->getSequence() == sequence) {
			break;
		}
		node = node->next;
	}

	if(node == nullptr) {
		return;
	}

	// delete the next node
	PacketNode* deletedNode = node->next;
	node->next = deletedNode->next;
	delete deletedNode->packet;
	delete deletedNode;
}
