#pragma once

#include <string>

using namespace std;

namespace binds {
	enum Axes { // corresponds to GLFW axes
		LEFT_AXIS_X = 0,
		LEFT_AXIS_Y = 1,
		RIGHT_AXIS_X = 2,
		RIGHT_AXIS_Y = 3,
		MOUSE_AXIS_X = 4,
		MOUSE_AXIS_Y = 5,
	};

	enum GamepadButtons {
		INVALID_BUTTON,
		A_BUTTON,
		B_BUTTON,
		X_BUTTON,
		Y_BUTTON,
		D_PAD_UP,
		D_PAD_DOWN,
		D_PAD_LEFT,
		D_PAD_RIGHT,
		SPECIAL_LEFT,
		SPECIAL_RIGHT,
		LEFT_BUTTON,
		RIGHT_BUTTON,
		LEFT_TRIGGER,
		RIGHT_TRIGGER,
	};

	enum Action {
		RELEASE,
		PRESS,
	};
	
	struct Keybind {
		string bind;
	};
}
