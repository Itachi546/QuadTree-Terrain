#pragma once

#pragma once

#include "input/keyboard.h"
#include "input/mouse.h"
#include "graphics_api.h"


#include <stdint.h>
#include <functional>
#include <memory>

class Swapchain;
enum class WindowType : uint8_t
{
	WINDOWED = 0,
	FULLSCREEN
};

class GraphicsWindow
{
public:

	virtual void run(double frameRate) = 0;

	virtual std::shared_ptr<Keyboard> get_keyboard() = 0;
	virtual std::shared_ptr<Mouse> get_mouse() = 0;

	virtual int get_width() = 0;
	virtual int get_height() = 0;


	virtual void set_on_update_frame(std::function<void()> handler)
	{
		_on_update_frame = handler;
	}

	virtual void set_on_pre_render_frame(std::function<void()> handler)
	{
		_on_pre_render_frame = handler;
	}

	virtual void set_on_render_frame(std::function<void()> handler)
	{
		_on_render_frame = handler;
	}

	virtual void set_on_post_render_frame(std::function<void()> handler)
	{
		_on_post_render_frame = handler;
	}

	virtual void set_on_resize(std::function<void(int, int)> handler)
	{
		_on_resize = handler;
	}

	// @TODO remove later
	virtual float get_frame_time() = 0;

	virtual ~GraphicsWindow() {}

protected:

	virtual void on_update_frame()
	{
		if (_on_update_frame)
			_on_update_frame();
	}

	virtual void on_pre_render_frame()
	{
		if (_on_pre_render_frame)
			_on_pre_render_frame();

	}

	virtual void on_render_frame()
	{
		if (_on_render_frame)
			_on_render_frame();
	}

	virtual void on_post_render_frame()
	{
		if (_on_post_render_frame)
			_on_post_render_frame();
	}

	virtual void on_resize(int width, int height)
	{
		_on_resize(width, height);
	}

protected:
	std::function<void()>  _on_render_frame;
	std::function<void()>  _on_post_render_frame;
	std::function<void()>  _on_pre_render_frame;
	std::function<void()>  _on_update_frame;
	std::function<void(int, int)> _on_resize;
};
