#pragma once

#include <cstddef>
#include <string>

#include <torquescript/ts.h>

using namespace std;

class GameObject {
	public:
		GameObject();

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
