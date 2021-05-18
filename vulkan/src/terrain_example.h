#pragma once

#include "example_base.h"
#include "light/cascaded_shadow.h"
#include "terrain/terrain_stream.h"
#include "terrain/terrain.h"


class TerrainExample : public ExampleBase
{
public:
	TerrainExample() : ExampleBase(1920, 1055)
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
		camera = CreateRef<Camera>();
		camera->set_aspect(float(m_window->get_width()) / float(m_window->get_height()));
		camera->set_position(glm::vec3(0.0f, 50.0f, 0.0f));
		scene->set_camera(camera);
		
		Entity* cube = scene->create_cube();
		cube->transform->scale *= 1.0f;
		cube->transform->position.z += 50.0f;
		cube->transform->position.y += 20.0f;

		PerlinGenerator generator = {};
		generator.frequency = 0.008f;
		generator.exponent = 4.0f;
		generator.octaves = 6;
		Ref<TerrainStream> stream = CreateRef<TerrainStream>(generator);
		terrain = CreateRef<Terrain>(m_context, stream);
		scene->set_terrain(terrain);
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
		m_context->set_graphics_pipeline(pipeline);
		scene->render(m_context);
		DebugDraw::render(m_context, scene->get_uniform_binding());
		m_context->end_renderpass();
		m_context->end();
	}

	void handle_input(float dt)
	{
		const float speed = 20.0f;
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
		camera->set_aspect(float(m_window->get_width()) / float(m_window->get_height()));
	}

	~TerrainExample()
	{
		terrain->destroy();
		scene->destroy();
		Device::destroy_pipeline(pipeline);
	}

private:
	Pipeline* pipeline;
	Entity* cube;
	std::shared_ptr<Scene> scene;
	Ref<Terrain> terrain;
	Ref<Camera> camera;
	float mouseX = 0.0f, mouseY = 0.0f;
};

ExampleBase* CreateTerrainExampleFn()
{
	return new TerrainExample();
}