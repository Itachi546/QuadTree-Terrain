#pragma once

#include "renderer/renderer.h"
#include "scene/gpu_memory.h"
#include "scene/scene.h"
#include "scene/camera.h"
#include "core/math.h"
#include "common/common.h"
#include <vector>

class ExampleBase
{
public:
	ExampleBase(int width, int height)
	{
		m_window = Device::create_window(width, height, "Hello Vulkan");
		m_context = Device::create_context(m_window);
		DebugDraw::init(m_context);
		m_window->set_on_render_frame([&]() { render(); });
		m_window->set_on_resize([&](int width, int height) { on_resize(width, height); });
	}

	void run()
	{
		m_window->run(60.0);
	}

	virtual ~ExampleBase()
	{
		GPU_MEMORY_POOL::GpuMemory::get_instance()->destroy();
		DebugDraw::destroy();
		Device::destroy_context(m_context);
		Device::destroy_window(m_window);
	}

protected:
	virtual void render() = 0;
	virtual void on_resize(int width, int height){}

	GraphicsWindow* m_window;
	Context* m_context;
	float angle = 0.0f;
};

