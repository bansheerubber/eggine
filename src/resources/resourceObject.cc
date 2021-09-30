#include "resourceObject.h"

#include "resourceManager.h"

resources::ResourceObject::ResourceObject(ResourceManager* manager, carton::Metadata* metadata) {
	this->manager = manager;
	this->metadata = metadata;
	if(manager != nullptr) {
		manager->objects.insert(this);
	}
}

void resources::ResourceObject::lease() {
	this->leases++;
}

void resources::ResourceObject::release() {
	this->leases--;

	if(this->leases < 0) {
		printf("negative leases on resource object\n");
		exit(1);
	}
}

int resources::ResourceObject::getLeaseCount() {
	return this->leases;
}
