#pragma once

#include "example_base.h"
#include "light/cascaded_shadow.h"

class CubeExample : public ExampleBase
{
public:
	CubeExample() : ExampleBase(1920, 1055)
	{
		std::string vertexCode = load_file("spirv/main.vert.spv");
		ASSERT(vertexCode.size() % 4 == 0);
		std::string fragmentCode = load_file("spirv/main.frag.spv");
		ASSERT(fragmentCode.size() % 4 == 0);

		PipelineDescription pipelineDesc = {};
		ShaderDescription shaderDescription[2] = {};
		shaderDescription[0].shaderStage = ShaderStage::Vertex;
		shaderDescription[0].code = vertexCode;
		shaderDescription[0].sizeInByte = static_cast<uint32_t>(vertexCode.size());
		shaderDescription[1].shaderStage = ShaderStage::Fragment;
		shaderDescription[1].code = fragmentCode;
		shaderDescription[1].sizeInByte = static_cast<uint32_t>(fragmentCode.size());
		pipelineDesc.shaderStageCount = 2;
		pipelineDesc.shaderStages = shaderDescription;
		pipelineDesc.renderPass = m_context->get_global_renderpass();
		pipelineDesc.rasterizationState.depthTestFunction = CompareOp::LessOrEqual;
		pipelineDesc.rasterizationState.enableDepthTest = true;
		pipelineDesc.rasterizationState.faceCulling = FaceCulling::Back;
		pipelineDesc.rasterizationState.topology = Topology::Triangle;
		pipeline = Device::create_pipeline(pipelineDesc);

		scene = std::make_shared<Scene>("Hello World", m_context);
		cube = scene->create_cube();
		cube->transform->scale *= 0.5f;

		Entity* plane = scene->create_plane();
		plane->transform->position.y -= 0.5f;
		plane->transform->scale = glm::vec3(10.0f);

		Entity* sphere = scene->create_sphere();
		sphere->transform->position += glm::vec3(1.0f, 0.0f, 0.0f);
		sphere->transform->scale *= 0.5f;

		Entity* cube2 = scene->create_cube();
		cube2->transform->position -= glm::vec3(-2.0f, 0.0f, 0.0f);
		cube2->transform->scale *= 0.5f;


		camera = CreateRef<Camera>();
		camera->set_aspect(float(m_window->get_width()) / float(m_window->get_height()));
		scene->set_camera(camera);
	}

	void render() override
	{
		float dt = m_window->get_frame_time();
		handle_input(dt);
		scene->update(m_context, dt);

		m_context->begin();
		scene->prepass(m_context);

		m_context->set_clear_color(0.0f, 0.0f, 0.0f, 1.0f);
		m_context->set_clear_depth(1.0f);
		m_context->begin_renderpass(nullptr, nullptr);
		m_context->set_graphics_pipeline(pipeline);
		scene->render(m_context);
		DebugDraw::render(m_context, scene->get_uniform_binding());
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
		if (mouse->is_down(Button::Left))
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

	~CubeExample()
	{
		scene->destroy();
		Device::destroy_pipeline(pipeline);
	}

private:
	Pipeline* pipeline;
	Entity* cube;
	std::shared_ptr<Scene> scene;
	Ref<Camera> camera;
	float mouseX = 0.0f, mouseY = 0.0f;
};

ExampleBase* CreateCubeExampleFn()
{
	return new CubeExample();
}