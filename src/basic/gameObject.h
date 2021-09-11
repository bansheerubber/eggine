#pragma once

#include <cstddef>

class GameObject {
	public:
		GameObject();

		int operator==(const GameObject &other) {
			return this->id == other.id;
		}
	
	protected:
		size_t id;
		static size_t maxId;
};
