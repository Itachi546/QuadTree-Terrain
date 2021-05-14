#pragma once

#include "example_base.h"
#include "terrain/terrain.h"
#include "terrain/terrain_stream.h"
#include "light/cascaded_shadow.h"

class TerrainExample : public ExampleBase
{
public:
	TerrainExample() : ExampleBase(1920, 1055)
	{
		scene = CreateRef<Scene>("PhysicsTest", m_context);

		uint32_t width = m_window->get_width();
		uint32_t height = m_window->get_height();
		camera = CreateRef<Camera>();
		camera->set_aspect(float(width) / float(height));
		camera->set_position(glm::vec3(0.0f, 80.0f, 50.0f));
		scene->set_camera(camera);

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

		Entity* cube = scene->create_cube();
		cube->transform->position = glm::vec3(30.0f, 50.0f, 80.0f);
		cube->transform->scale = glm::vec3(0.5f, 1.0f, 0.5f);
		Ref<TerrainStream> stream = CreateRef<TerrainStream>("assets/heightmap2.png");
		terrain = CreateRef<Terrain>(m_context, stream);
	}

	~TerrainExample()
	{
		Device::destroy_pipeline(pipeline);
		terrain->destroy();
		scene->destroy();
	}

private:
	void render() override
	{
		float dt = m_window->get_frame_time();
		handle_input(dt);
		scene->update(m_context, dt);

		std::shared_ptr<Mouse> mouse = m_window->get_mouse();
		m_context->begin();

		m_context->set_clear_color(0.0f, 0.0f, 0.0f, 1.0f);
		m_context->set_clear_depth(1.0f);
		m_context->begin_renderpass(nullptr, nullptr);
		m_context->set_graphics_pipeline(pipeline);
		scene->render(m_context);
		terrain->render(m_context, scene->get_uniform_binding());
		DebugDraw::render(m_context, scene->get_uniform_binding());
		m_context->end_renderpass();
		m_context->end();
	}

	void handle_input(float dt)
	{
		const float speed = 5.0f;
		auto keyboard = m_window->get_keyboard();
		if (keyboard->is_down(KeyCode::W))
			camera->walk(-dt * speed);
		else if (keyboard->is_down(KeyCode::S))
			camera->walk(dt * speed);
		if (keyboard->is_down(KeyCode::A))
			camera->strafe(-dt * speed);
		else if (keyboard->is_down(KeyCode::D))
			camera->strafe(dt * speed);
		if (keyboard->is_down(KeyCode::Q))
			camera->lift(dt * speed);
		else if (keyboard->is_down(KeyCode::E))
			camera->lift(-dt * speed);

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

		camera->set_aspect(float(width) / float(height));
	}

	Ref<Scene>  scene;
	Ref<Camera> camera;
	Ref<Terrain> terrain;
	float mouseX = 0.0f, mouseY = 0.0f;
	Pipeline* pipeline;
};

ExampleBase* CreateTerrainExampleFn()
{
	return new TerrainExample();
}