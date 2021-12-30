#pragma once

#include <cstddef>
#include <eggscript/egg.h>
#include <string>

#include "gameObject.h.gen"
#include "../engine/keybind.h"

class GameObject {
	public:
		GameObject();
		virtual ~GameObject();
		
		virtual GameObjectType getType() {
			return INVALID;
		}

		virtual void onBind(std::string &bind, binds::Action action) {}
		virtual void onAxis(std::string &bind, double axis) {};

		int operator==(const GameObject &other) {
			return this->id == other.id;
		}

		esObjectReferencePtr reference = nullptr;
	
	protected:
		uint64_t id;
		static uint64_t maxId;
};
