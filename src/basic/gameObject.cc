#include "gameObject.h"

size_t GameObject::maxId = 0;

GameObject::GameObject() {
	this->id = this->maxId++;
}
