#pragma once

#include "example_base.h"
#include "debug/gizmo.h"
#include "core/ray.h"
#include "physics/physics.h"

class GizmoExample : public ExampleBase
{
public:
	GizmoExample() : ExampleBase(1920, 1055)
	{
		scene = CreateRef<Scene>("GizmoExample", m_context);

		gizmo = CreateRef<Gizmo>();
		gizmo->init(m_context, m_window->get_width(), m_window->get_height());
		gizmo->set_operation(Operation::Translate);

		{
			Entity* plane = scene->create_plane();
			plane->transform->position.y -= 1.0f;
			plane->transform->scale = glm::vec3(10.0f, 1.0f, 10.0f);
			Ref<Rigidbody> rb = CreateRef<Rigidbody>();
			rb->transform = plane->transform;
			rb->collider = CreateRef<PlaneCollider>(glm::vec3(0.0f, 1.0f, 0.0f), plane->transform->position.y);
			physicsSystem->add_rigid_body(rb);
		}

		{
			Entity* sphere = scene->create_sphere();
			sphere->transform->position -= glm::vec3(2.0f, 0.0f, 0.0f);

			Ref<Rigidbody> rb = CreateRef<Rigidbody>();
			rb->transform = sphere->transform;
			rb->collider = CreateRef<SphereCollider>();
			physicsSystem->add_rigid_body(rb);
		}

		{
			Entity* sphere = scene->create_sphere();
			sphere->transform->position -= glm::vec3(0.0f, 0.0f, 0.0f);
			sphere->transform->scale *= 0.2f;
			Ref<Rigidbody> rb = CreateRef<Rigidbody>();
			rb->transform = sphere->transform;
			rb->collider = CreateRef<SphereCollider>();
			physicsSystem->add_rigid_body(rb);
		}


		uint32_t width = m_window->get_width();
		uint32_t height = m_window->get_height();
		camera = CreateRef<Camera>();
		camera->set_aspect(float(width) / float(height));
		scene->set_camera(camera);
	}

	~GizmoExample()
	{
		scene->destroy();
		gizmo->destroy();
	}

private:

	void update(float dt) override
	{
		handle_input(dt);
		scene->update(m_context, dt);
		gizmo->manipulate(camera->get_projection(), camera->get_view(), float(mouseX), float(mouseY), mouse->is_down(Button::Left));
		physicsSystem->draw_manifolds();
	}

	void render() override
	{

		m_context->begin();
		gizmo->prepass(m_context, scene->get_uniform_binding());
		scene->prepass(m_context);

		m_context->set_clear_color(0.0f, 0.0f, 0.0f, 1.0f);
		m_context->set_clear_depth(1.0f);
		m_context->begin_renderpass(nullptr, nullptr);
		scene->render(m_context);
		gizmo->render(m_context);
		m_context->end_renderpass();
		m_context->end();

		RayHit hit = {};
		if (!gizmo->is_active() && mouse->is_down(Button::Left))
		{
			glm::vec2 mousePos;
			mouse->get_mouse_position(&mousePos.x, &mousePos.y);
			if (scene->cast_ray(camera->generate_ray(mousePos, glm::vec2(m_window->get_width(), m_window->get_height())), hit))
			{
				gizmo->set_active(hit.entity);
			}
		}
	}

	void handle_input(float dt)
	{
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

		if (keyboard->is_down(KeyCode::R))
			gizmo->set_operation(Operation::Rotate);
		else if(keyboard->is_down(KeyCode::T))
			gizmo->set_operation(Operation::Translate);
		else if (keyboard->is_down(KeyCode::Y))
			gizmo->set_operation(Operation::Scale);


		auto mouse = m_window->get_mouse();
		float x, y;
		mouse->get_mouse_position(&x, &y);
		if (mouse->is_down(Button::Middle))
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

		camera->set_aspect(float(width) / float(height));
		gizmo->on_resize(width, height);
	}

	Entity* cube;

	Ref<Scene>  scene;
	Ref<Camera> camera;
	Ref<Gizmo>  gizmo;
	float mouseX = 0.0f, mouseY = 0.0f;
};

ExampleBase* CreateGizmoExampleFn()
{
	return new GizmoExample();
}