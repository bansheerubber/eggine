#include "network.h"

#include <algorithm>

#include "../basic/remoteObject.h"

void network::Network::tick() {
	Stream stream;
	for(RemoteObject* remoteObject: this->remoteObjects) {
		if(remoteObject->hasUpdate()) {
			remoteObject->pack(stream);
			this->send(stream);
		}
	}
}

void network::Network::send(Stream &stream) {
	if(stream.buffer.head) {
		printf("send %d bytes\n", stream.buffer.head);
	}
	stream.flush();
}

void network::Network::addRemoteObject(RemoteObject* remoteObject) {
	this->remoteObjects.push_back(remoteObject);
}

void network::Network::removeRemoteObject(RemoteObject* remoteObject) {
	this->remoteObjects.erase(find(this->remoteObjects.begin(), this->remoteObjects.end(), remoteObject));
}
