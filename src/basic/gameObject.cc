#include "gameObject.h"

uint64_t GameObject::maxId = 0;

GameObject::GameObject() {
	this->id = this->maxId++;
}

GameObject::~GameObject() {
	
}
