#pragma once

#include "example_base.h"
#include "light/directional_light.h"
#include "core/frustum.h"
#include "utils/skybox.h"
#include "terrain/grass.h"
#include "terrain/perlin_noise.h"

float get_height(siv::PerlinNoise noise, double x, double y, float maxHeight, float frequency)
{
	return static_cast<float>((noise.accumulatedOctaveNoise2D_0_1(x * frequency, y * frequency, 5) * 2.0 - 1.0) * maxHeight);
}

class GrassExample : public ExampleBase
{
public:
	GrassExample() : ExampleBase(1920, 1055)
	{
		scene = std::make_shared<Scene>("Hello World", m_context);
		//cube = scene->create_cube();
		//cube->transform->scale *= 0.5f;
		
		Ref<Mesh> mesh = CreateRef<Mesh>();
		// Terrain Width and height
		int VERTEX_COUNT = 1024;
		int width = 100;
		int height = 100;

		float mX = 1.0f / float(width);
		float mZ = 1.0f / float(height);

		siv::PerlinNoise noise;

		std::vector<Vertex> vertices;
		const float frequency = 0.05f;
		const float maxHeight = 5.0f;
		for (int z = 0; z <= VERTEX_COUNT; ++z)
		{
			double dz = z / double(VERTEX_COUNT);
			for (int x = 0; x <= VERTEX_COUNT; ++x)
			{
				double dx = x / double(VERTEX_COUNT);
				float h = get_height(noise, dx * width, dz * height, maxHeight, frequency);
				float a = get_height(noise, dx * width + 1.0, dz * height, maxHeight, frequency);
				float b = get_height(noise, dx * width - 1.0, dz * height, maxHeight, frequency);
				float c = get_height(noise, dx * width, dz * height + 1.0, maxHeight, frequency);
				float d = get_height(noise, dx * width, dz * height - 1.0, maxHeight, frequency);

				Vertex vertex;
				vertex.position = glm::vec3(dx * width, h, dz * height);
				vertex.normal = glm::normalize(glm::vec3(a - b, h, d - c));
				vertex.normal = vec3(0.0f, 1.0f, 0.0f);
				vertices.push_back(vertex);
			}
		}

		mesh->vertices = vertices;

		std::vector<unsigned int> indices;
		for (int z = 0; z < VERTEX_COUNT; ++z)
		{
			for (int x = 0; x < VERTEX_COUNT; ++x)
			{
				int i0 = z * (VERTEX_COUNT + 1) + x;
				int i1 = i0 + 1;
				int i2 = i0 + (VERTEX_COUNT + 1);
				int i3 = i2 + 1;

				indices.push_back(i2);
				indices.push_back(i1);
				indices.push_back(i0);

				indices.push_back(i2);
				indices.push_back(i3);
				indices.push_back(i1);
			}
		}
		mesh->indices = indices;
		mesh->finalize(m_context);

		plane = scene->create_entity("terrian");
		plane->transform->position -= glm::vec3(width * 0.5f, 0.5f, height * 0.5f);
		plane->transform->scale = glm::vec3(1.0f);
		plane->material->albedo = glm::vec3(0.01f, 0.3f, 0.01f);
		plane->mesh = mesh;

		cube = scene->create_cube();
		cube->transform->position += glm::vec3(0.0f, 0.5f, 5.0f);
		cube->transform->scale *= glm::vec3(0.5, 1.0, 0.5);
		cube->material->albedo = glm::vec3(1.0f, 0.1f, 0.1f);
		cube->material->metallic = 0.5f;
		cube->material->roughness = 0.5f;

		camera = CreateRef<Camera>();
		camera->set_aspect(float(m_window->get_width()) / float(m_window->get_height()));
		scene->set_camera(camera);
		scene->get_directional_light()->set_cast_shadow(false);

		grass = CreateRef<Grass>(m_context);
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

		ShaderBindings* bindings[] = {
			scene->get_uniform_binding(),
			scene->get_light_binding()
		};
		grass->render(m_context, plane, bindings, ARRAYSIZE(bindings), m_totalTime);

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

	~GrassExample()
	{
		grass->destroy();
		scene->destroy();
	}

private:
	Entity* plane;
	Entity* cube;
	std::shared_ptr<Scene> scene;
	Ref<Camera> camera;
	Ref<Grass> grass;
	float mouseX = 0.0f, mouseY = 0.0f;
};

ExampleBase* CreateGrassExampleFn()
{
	return new GrassExample();
}