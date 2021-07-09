#pragma once

#include "example_base.h"
#include "light/cascaded_shadow.h"
#include "core/frustum.h"
#include "utils/skybox.h"
#include "scene/material.h"

class PBRExample : public ExampleBase
{
public:
	PBRExample() : ExampleBase(1920, 1055)
	{
		scene = std::make_shared<Scene>("Hello World", m_context);
		//cube = scene->create_cube();
		//cube->transform->scale *= 0.5f;

		camera = CreateRef<Camera>();
		camera->set_aspect(float(m_window->get_width()) / float(m_window->get_height()));
		scene->set_camera(camera);
		scene->get_directional_light()->set_cast_shadow(false);
		skybox = CreateRef<Skybox>(m_context, "assets/hdri/newport_loft.hdr");
		scene->set_skybox(skybox);

		const float radius = 1.0f;

		int nRows = 7;
		int nCols = 7;
		for (int y = 0; y < nRows; ++y)
		{
			float metallic = float(y) / nRows;
			for (int x = 0; x < nCols; ++x)
			{
				Entity* sphere = scene->create_sphere();
				sphere->transform->position += glm::vec3(float(x) * radius, float(y) * radius, 0.0f);
				sphere->transform->scale = glm::vec3(0.45f);
				sphere->material->albedo = glm::vec3(1.0f, 0.01f, 0.01f);
				sphere->material->metallic = metallic;
				sphere->material->roughness = glm::clamp(float(x) / nCols, 0.05f, 1.0f);
			}
		}
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

		m_context->set_clear_color(0.5f, 0.7f, 0.9f, 1.0f);
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

	~PBRExample()
	{
		scene->destroy();
	}

private:
	Ref<Skybox> skybox;
	Entity* cube;
	std::shared_ptr<Scene> scene;
	Ref<Camera> camera;
	float mouseX = 0.0f, mouseY = 0.0f;
};

ExampleBase* CreatePBRExampleFn()
{
	return new PBRExample();
}