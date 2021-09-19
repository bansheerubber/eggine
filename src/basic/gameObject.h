#pragma once

#include <cstddef>
#include <string>

#include <torquescript/ts.h>

using namespace std;

enum GameObjectType {
	INVALID = 0,
	CHUNK_CONTAINER = 1,
};

class GameObject {
	public:
		GameObject();
		
		virtual GameObjectType getType() {
			return INVALID;
		}

		virtual void onBindPress(string &bind) {}
		virtual void onBindRelease(string &bind) {}
		virtual void onBindHeld(string &bind, double deltaTime) {}

		int operator==(const GameObject &other) {
			return this->id == other.id;
		}

		tsObjectReferencePtr reference = nullptr;
	
	protected:
		size_t id;
		static size_t maxId;
};
