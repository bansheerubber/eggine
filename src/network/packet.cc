#include "packet.h"

network::Packet::Packet() {
	this->stream.writeNumber<char>(0);
	this->stream.writeNumber<unsigned int>(0);
	this->stream.writeNumber<unsigned int>(0);
	this->stream.writeNumber<unsigned long>(0);
}

void network::Packet::setType(PacketType type) {
	size_t oldHead = this->stream.buffer.head;
	this->stream.buffer.head = 0;
	this->stream.writeNumber((char)type);
	this->stream.buffer.head = oldHead;
	this->type = type;
}

network::PacketType network::Packet::getType() {
	return this->type;
}

unsigned int network::Packet::getSequence() {
	return this->sequence;
}

void network::Packet::setHeader(unsigned int sequence, unsigned int lastAckedSequence, unsigned long ackMask) {
	size_t oldHead = this->stream.buffer.head;
	this->stream.buffer.head = 0;
	this->stream.writeNumber((char)this->type);
	this->stream.writeNumber(sequence);
	this->stream.writeNumber(lastAckedSequence);
	this->stream.writeNumber(ackMask);
	this->stream.buffer.head = oldHead;

	this->sequence = sequence;
}
