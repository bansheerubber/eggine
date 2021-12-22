#pragma once

namespace network {
	#define EGGINE_PACKET_SIZE 32768
	
	enum StreamType {
		REMOTE_OBJECT_UPDATE,
	};

	enum PacketType {
		INVALID_PACKET,
		DROPPABLE_PACKET,
		MAX_PACKET,
	};
};