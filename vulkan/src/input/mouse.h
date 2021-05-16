#pragma once

#include "input_keys.h"
class Mouse
{

public:
	bool is_down(Button button)
	{
		return _buttons[int(button)];
	}

	void on_key_down(Button button)
	{
		_buttons[static_cast<int>(button)] = true;
	}

	void on_key_up(Button button)
	{
		_buttons[static_cast<int>(button)] = false;
	}

	void set_mouse_position(float x, float y)
	{
		_x = x;
		_y = y;
	}

	void get_mouse_position(float* x, float* y)
	{
		*x = _x;
		*y = _y;
	}

private:
	bool _buttons[10];
	float _x;
	float _y;
};