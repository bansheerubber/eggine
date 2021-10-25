#pragma once

#include <string>

using namespace std;

namespace binds {
	enum Axes { // corresponds to GLFW axes
		LEFT_AXIS_X = 0,
		LEFT_AXIS_Y = 1,
		RIGHT_AXIS_X = 2,
		RIGHT_AXIS_Y = 3,
	};
	
	struct Keybind {
		string bind;
	};
}
