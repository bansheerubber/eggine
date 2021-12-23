#pragma once

#include <netinet/in.h>
#include <tsl/robin_map.h>
#include <sys/socket.h>
#include <vector>

#include "stream.h"
#include "types.h"

namespace std {
	template<>
	struct hash<sockaddr_in6> {
		size_t operator()(sockaddr_in6 const& source) const noexcept {
			size_t hash =
				(unsigned long)source.sin6_addr.s6_addr[0]
				| ((unsigned long)source.sin6_addr.s6_addr[1] << (unsigned long)8)
				| ((unsigned long)source.sin6_addr.s6_addr[2] << (unsigned long)16)
				| ((unsigned long)source.sin6_addr.s6_addr[3] << (unsigned long)24)
				| ((unsigned long)source.sin6_addr.s6_addr[4] << (unsigned long)32)
				| ((unsigned long)source.sin6_addr.s6_addr[5] << (unsigned long)40)
				| ((unsigned long)source.sin6_addr.s6_addr[6] << (unsigned long)56)
				| ((unsigned long)source.sin6_addr.s6_addr[7] << (unsigned long)48);
			size_t second = 
				(unsigned long)source.sin6_addr.s6_addr[8]
				| ((unsigned long)source.sin6_addr.s6_addr[9] << (unsigned long)8)
				| ((unsigned long)source.sin6_addr.s6_addr[10] << (unsigned long)16)
				| ((unsigned long)source.sin6_addr.s6_addr[11] << (unsigned long)24)
				| ((unsigned long)source.sin6_addr.s6_addr[12] << (unsigned long)32)
				| ((unsigned long)source.sin6_addr.s6_addr[13] << (unsigned long)40)
				| ((unsigned long)source.sin6_addr.s6_addr[14] << (unsigned long)56)
				| ((unsigned long)source.sin6_addr.s6_addr[15] << (unsigned long)48);
			hash = hash ^ (second + 0x9e3779b9 + (hash << 6) + (hash >> 2));
			return hash ^ (source.sin6_port + 0x9e3779b9 + (hash << 6) + (hash >> 2));
    }
	};

	template<>
	struct equal_to<sockaddr_in6> {
		bool operator()(const sockaddr_in6& x, const sockaddr_in6& y) const {
			for(unsigned int i = 0; i < 16; i++) {
				if(x.sin6_addr.s6_addr[i] != y.sin6_addr.s6_addr[i]) {
					return false;
				}
			}

			return x.sin6_port == y.sin6_port;
		}
	};
};

namespace network {
	#define EGGINE_NETWORK_UDP_MESSAGE_AMOUNT 32
	
	class Network {
		public:
			Network();
			~Network();

			bool isActive();

			void open();
			void close();
			void accept();

			void sendInitialData(class Connection* connection);

			void tick();
			void recv();
			
			void addRemoteObject(class RemoteObject* remoteObject);
			void removeRemoteObject(class RemoteObject* remoteObject);
			class RemoteObject* getRemoteObject(remote_object_id id);

			int getUDPSocket();

			bool isServer();
			bool isClient();
		
		private:
			unsigned long highestRemoteId = 0;
			int tcpSocket = -1;
			int udpSocket = -1;
			std::vector<class RemoteObject*> remoteObjects;
			tsl::robin_map<remote_object_id, class RemoteObject*> idToRemoteObject;
			std::vector<class Connection*> clients;

			tsl::robin_map<sockaddr_in6, class Connection*> udpAddressToConnection;
			tsl::robin_map<unsigned long, class Connection*> secretToConnection;

			struct {
				Stream streams[EGGINE_NETWORK_UDP_MESSAGE_AMOUNT];
				iovec scatterGather[32];
				cmsghdr controls[32];
				sockaddr_in6 addresses[EGGINE_NETWORK_UDP_MESSAGE_AMOUNT];
				mmsghdr headers[EGGINE_NETWORK_UDP_MESSAGE_AMOUNT];
			} udp;

			unsigned long long frog;
	};
};
