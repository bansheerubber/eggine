#pragma once

#include <netinet/in.h>
#include <tsl/robin_map.h>
#include <sys/socket.h>
#include <vector>

#include "client.h"
#include "stream.h"
#include "types.h"

namespace std {
	template<>
	struct hash<sockaddr_in6> {
		size_t operator()(sockaddr_in6 const& source) const noexcept {
			size_t hash = *((size_t*)&source.sin6_addr.s6_addr[0]);
			size_t second = *((size_t*)&source.sin6_addr.s6_addr[8]);
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

	enum NetworkMode {
		NETWORK_INACTIVE,
		NETWORK_SERVER,
		NETWORK_CLIENT,
	};
	
	class Network {
		public:
			Network();
			~Network();

			Client client;

			inline std::string getChecksum() {
				// ##1 remote_object_checksum
			}

			const IPAddress getHostIPAddress();

			void openServer();
			void closeServer();
			void acceptServer();

			void openClient();
			void closeClient();

			void tick();
			void receive();
			
			void addRemoteObject(class RemoteObject* remoteObject);
			void removeRemoteObject(class RemoteObject* remoteObject);
			class RemoteObject* getRemoteObject(remote_object_id id);

			int getUDPSocket();

			bool isServer(); // whether or not the network is acting as a server
			bool isClient(); // whether or not the network is acting as a client

			void removeConnection(class Connection* connection);
			unsigned int getConnectionCount();
		
		private:
			IPAddress ip;
			
			NetworkMode mode = NETWORK_INACTIVE;
			
			// bookkeeping for remote objects
			uint64_t highestRemoteId = 0;
			int tcpSocket = -1;
			std::vector<class RemoteObject*> remoteObjects;
			tsl::robin_map<remote_object_id, class RemoteObject*> idToRemoteObject;

			std::vector<class Connection*> connections;
			tsl::robin_map<sockaddr_in6, class Connection*> udpAddressToConnection;
			tsl::robin_map<uint64_t, class Connection*> secretToConnection;

			struct {
				Stream streams[EGGINE_NETWORK_UDP_MESSAGE_AMOUNT];
				iovec scatterGather[32];
				cmsghdr controls[32];
				sockaddr_in6 addresses[EGGINE_NETWORK_UDP_MESSAGE_AMOUNT];
				mmsghdr headers[EGGINE_NETWORK_UDP_MESSAGE_AMOUNT];
				int socket;
			} udp;

			uint64_t frog;
			unsigned int sent = 0;

			void sendInitialData(class Connection* connection);
	};
};
