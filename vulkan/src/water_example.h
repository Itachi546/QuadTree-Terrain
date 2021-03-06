#pragma once

#include "example_base.h"
#include "core/frustum.h"
#include "light/directional_light.h"
#include "water/water.h"

class WaterExample : public ExampleBase
{
public:
	WaterExample() : ExampleBase(1920, 1055)
	{

		scene = std::make_shared<Scene>("WaterExample", m_context);
		water = CreateRef<Water>(m_context);
		scene->set_water(water);

		cube = scene->create_cube();
		cube->transform->position = glm::vec3(128.0f, 8.0f, 120.0f);
		cube->transform->scale = glm::vec3(4.0f);

		camera = CreateRef<Camera>();
		camera->set_aspect(float(m_window->get_width()) / float(m_window->get_height()));
		camera->set_position(glm::vec3(128.0f, 20.0f, 128.0f));
		scene->set_camera(camera);
		scene->get_directional_light()->set_cast_shadow(false);
	}

	void update(float dt) override
	{
		static float angle = 0.0f;
		angle += dt;
		cube->transform->set_rotation(glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f)), angle);
		handle_input(dt);
		scene->update(m_context, dt);
		water->update(m_context, dt);
	}

	void render() override
	{

		m_context->begin();
		scene->prepass(m_context);

		m_context->set_clear_color(0.5f, 0.7f, 0.9f, 1.0f);
		m_context->set_clear_depth(1.0f);
		m_context->begin_renderpass(nullptr, nullptr);
		scene->render(m_context);
		DebugDraw::immediate_draw_textured_quad(m_context, water->debugBindings, glm::vec4(0.9f, 0.5f, 0.1f, 0.1f));
		m_context->end_renderpass();
		m_context->end();
	}

	void handle_input(float dt)
	{
		dt *= 10.0f;
		auto keyboard = m_window->get_keyboard();
		if (keyboard->is_down(KeyCode::W))
			camera->walk(-dt);
		else if (keyboard->is_down(KeyCode::S))
			camera->walk(dt);
		if (keyboard->is_down(KeyCode::A))
			camera->strafe(-dt);
		else if (keyboard->is_down(KeyCode::D))
			camera->strafe(dt);
		if (keyboard->is_down(KeyCode::Q))
			camera->lift(dt);
		else if (keyboard->is_down(KeyCode::E))
			camera->lift(-dt);

		auto mouse = m_window->get_mouse();
		float x, y;
		mouse->get_mouse_position(&x, &y);
		bool isUIActive = ImGui::IsAnyItemActive() || ImGui::IsAnyItemFocused() || ImGui::IsAnyItemHovered();
		if (mouse->is_down(Button::Left) && !isUIActive)
		{
			float dx = x - mouseX;
			float dy = y - mouseY;
			camera->rotate(dy, -dx, dt);
		}
		mouseX = x;
		mouseY = y;
	}

	inline void on_resize(int width, int height) override
	{
		if (width == 0 || height == 0)
			return;
		camera->set_aspect(float(m_window->get_width()) / float(m_window->get_height()));
	}

	~WaterExample()
	{
		water->destroy();
		scene->destroy();

	}

private:
	Entity* cube;
	std::shared_ptr<Scene> scene;
	Ref<Camera> camera;
	float mouseX = 0.0f, mouseY = 0.0f;

	Ref<Water> water;
};

ExampleBase* CreateWaterExampleFn()
{
	return new WaterExample();
}