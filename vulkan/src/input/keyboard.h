#pragma once

#include "input_keys.h"

#include <stdint.h>

enum class KeyEventType : uint8_t
{
	UP = 0,
	DOWN
};

struct KeyboardEvent
{
	KeyEventType type;
	KeyCode key;
};

class Keyboard
{

public:
	bool is_down(KeyCode key)
	{
		return _keys[int(key)];
	}

	virtual void on_key_down(KeyCode key)
	{
		_keys[static_cast<int>(key)] = true;
	}

	virtual void on_key_up(KeyCode key)
	{
		_keys[static_cast<int>(key)] = false;
	}

private:
	bool _keys[512];
};