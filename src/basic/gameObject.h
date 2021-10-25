#pragma once

#include <cstddef>
#include <eggscript/egg.h>
#include <string>

#include "gameObject.h.gen"

using namespace std;

class GameObject {
	public:
		GameObject();
		
		virtual GameObjectType getType() {
			return INVALID;
		}

		virtual void onBindPress(string &bind) {}
		virtual void onBindRelease(string &bind) {}
		virtual void onBindHeld(string &bind, double deltaTime) {}
		virtual void onAxis(string &bind, double axis) {};

		int operator==(const GameObject &other) {
			return this->id == other.id;
		}

		esObjectReferencePtr reference = nullptr;
	
	protected:
		size_t id;
		static size_t maxId;
};
