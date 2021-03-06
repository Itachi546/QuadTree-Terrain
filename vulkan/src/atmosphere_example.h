#pragma once

#include "example_base.h"
#include "light/directional_light.h"
#include "atmosphere/atmosphere.h"
#include "renderer/gpu_query.h"

class AtmosphereExample : public ExampleBase
{
public:
	AtmosphereExample() : ExampleBase(1920, 1055, false)
	{
		scene = std::make_shared<Scene>("AtmosphereExample", m_context);
		//cube = scene->create_cube();
		//cube->transform->scale *= 0.5f;

		Entity* plane = scene->create_plane();
		plane->transform->position.y -= 0.5f;
		plane->transform->scale = glm::vec3(100.0f);

		Entity* sphere = scene->create_sphere();
		sphere->transform->position += glm::vec3(1.0f, 0.0f, 0.0f);
		sphere->transform->scale *= 0.5f;

		cube = scene->create_cube();
		cube->transform->position -= glm::vec3(-2.0f, 0.0f, 0.0f);
		cube->transform->scale *= 0.5f;

		camera = CreateRef<Camera>();
		camera->set_aspect(float(m_window->get_width()) / float(m_window->get_height()));
		scene->set_camera(camera);
		//scene->get_directional_light()->set_cast_shadow(false);
	}

	void update(float dt) override
	{
		handle_input(dt);
		scene->update(m_context, dt);
	}

	void render() override
	{
		m_context->begin();
		scene->prepass(m_context);
		m_context->set_clear_color(0.0f, 0.0f, 0.0f, 1.0f);
		m_context->set_clear_depth(1.0f);
		m_context->begin_renderpass(nullptr, nullptr);
		scene->render(m_context);
		m_context->end_renderpass();
		m_context->end();
	}

	void handle_input(float dt)
	{
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

	~AtmosphereExample()
	{
		scene->destroy();
	}

private:
	Entity* cube;
	std::shared_ptr<Scene> scene;
	Ref<Camera> camera;
	float mouseX = 0.0f, mouseY = 0.0f;
};

ExampleBase* CreateAtmosphereExampleFn()
{
	return new AtmosphereExample();
}