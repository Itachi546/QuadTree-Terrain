#pragma once

#include "renderer/renderer.h"
#include "scene/gpu_memory.h"
#include "scene/scene.h"
#include "scene/camera.h"
#include "core/math.h"
#include "common/common.h"
#include "physics/physics_system.h"
#include "imgui/imgui.h"
#include "renderer/context.h"
#include <vector>

class ExampleBase
{
public:
	ExampleBase(int width, int height)
	{
		m_window = Device::create_window(width, height, "Hello Vulkan");
		m_context = Device::create_context(m_window);
		DebugDraw::init(m_context);
		m_window->set_on_render_frame([&]() { _render(); });
		m_window->set_on_resize([&](int width, int height) { on_resize(width, height); });
		m_window->set_on_update_frame([&]() {_update(); });

		mouse = m_window->get_mouse();
		keyboard = m_window->get_keyboard();

		physicsSystem = CreateRef<PhysicsSystem>();
		physicsSystem->init();
	}

	void run()
	{
		m_window->run(60.0);
	}

	virtual void update(float dt) = 0;
	virtual void render() = 0;

	virtual ~ExampleBase()
	{
		physicsSystem->destroy();
		GPU_MEMORY_POOL::GpuMemory::get_instance()->destroy();
		DebugDraw::destroy();
		Device::destroy_context(m_context);
		Device::destroy_window(m_window);
	}

protected:
	void _render() 
	{ 
		m_context->acquire_swapchain_image();
		render(); 
		m_context->present();
	};
	virtual void _update() 
	{
		float dt = m_window->get_frame_time();
		if (ImGui::Begin("Statistics", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse))
		{
			ImGui::Text("frametime: %.3fms", m_dt * 1000.0f);
			ImGui::Text("framerate: %d", m_lastFPS);

			float gpuMemory = float(Device::get_total_memory_allocated()) / (1024.0f * 1024.0f);
			ImGui::Text("GPU Memory: %.1fMB", gpuMemory);
			ImGui::End();
		}

		physicsSystem->step(dt);
		update(dt); 

		m_frameCount++;
		m_elapsedTime += dt;
		if (m_elapsedTime > 1.0f)
		{
			m_lastFPS = m_frameCount;
			m_dt = m_elapsedTime / float(m_frameCount);
			m_elapsedTime = 0.0f;
			m_frameCount = 0;
		}

	}
	virtual void on_resize(int width, int height){}

	GraphicsWindow* m_window;
	Context* m_context;
	float angle = 0.0f;
	Ref<PhysicsSystem> physicsSystem;

	Ref<Mouse> mouse;
	Ref<Keyboard> keyboard;

	float m_elapsedTime = 0.0f;
	float m_dt = 1.0f / 60.0f;
	int m_frameCount = 0;
	int m_lastFPS = 60;
};

