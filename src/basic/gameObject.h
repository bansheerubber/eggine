#pragma once

#include <cstddef>
#include <string>

using namespace std;

class GameObject {
	public:
		GameObject();

		virtual void onBindPress(string &bind) {}
		virtual void onBindRelease(string &bind) {}
		virtual void onBindHeld(string &bind) {}

		int operator==(const GameObject &other) {
			return this->id == other.id;
		}
	
	protected:
		size_t id;
		static size_t maxId;
};
