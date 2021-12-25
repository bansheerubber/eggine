#pragma once

#include <cstddef>
#include <eggscript/egg.h>
#include <string>

#include "gameObject.h.gen"
#include "../engine/keybind.h"

using namespace std;

class GameObject {
	public:
		GameObject();
		virtual ~GameObject();
		
		virtual GameObjectType getType() {
			return INVALID;
		}

		virtual void onBind(string &bind, binds::Action action) {}
		virtual void onAxis(string &bind, double axis) {};

		int operator==(const GameObject &other) {
			return this->id == other.id;
		}

		esObjectReferencePtr reference = nullptr;
	
	protected:
		size_t id;
		static size_t maxId;
};
