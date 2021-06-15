#pragma once

#include "example_base.h"
#include "light/cascaded_shadow.h"
#include "terrain/terrain_stream.h"
#include "terrain/terrain.h"
#include "water/water.h"

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

		scene = std::make_shared<Scene>("TerrainExample", m_context);
		water = CreateRef<Water>(m_context);
		scene->set_water(water);
#if 0
		std::ifstream inFile("assets/heightmap.bin", std::ios::binary);
		int size[2];
		inFile.read(reinterpret_cast<char*>(size), sizeof(int) * 2);
		inFile.seekg(sizeof(int) * 2);

		uint32_t bufferSize = size[0] * size[1];
		float* buffer = new float[bufferSize];
		inFile.read(reinterpret_cast<char*>(buffer), bufferSize * sizeof(float));
		Ref<TerrainStream> stream = CreateRef<TerrainStream>(buffer, size[0], size[1]);
		uint32_t width = stream->get_width();
		uint32_t height = stream->get_height();
		water->set_translation(glm::vec3(width * 0.5f, -180.0f, height * 0.5f));
#else	
		Ref<TerrainStream> stream = CreateRef<TerrainStream>("assets/heightmap.png");
		uint32_t width = stream->get_width();
		uint32_t height = stream->get_height();
		water->set_translation(glm::vec3(width * 0.5f, -50.0f, height * 0.5f));
#endif
		terrain = CreateRef<Terrain>(m_context, stream);
		scene->set_terrain(terrain);
		cube = scene->create_cube();

		glm::vec3 cubePosition = glm::vec3(width * 0.68f, 0.0f, height * 0.55f);
		float h = 40.0f + terrain->get_height(cubePosition);
		cube->transform->position = glm::vec3(cubePosition.x, h, cubePosition.z);
		cube->transform->scale *= glm::vec3(5.0f, 10.0f, 5.0f);

		camera = CreateRef<Camera>();
		camera->set_aspect(float(m_window->get_width()) / float(m_window->get_height()));
		camera->set_position(glm::vec3(width * 0.65f, 50.0f, height * 0.65f));
		scene->set_camera(camera);
	}

	void update(float dt) override
	{
		handle_input(dt);
		glm::vec3 pos = camera->get_position();
		float height = terrain->get_height(pos);
		if ((pos.y - height) < 5.0f)
			camera->set_height(height + 5.0f);

		scene->update(m_context, dt);
		terrain->update(m_context, camera);
		water->update(m_context, dt);

		glm::vec2 mousePos = {};
		mouse->get_mouse_position(&mousePos.x, &mousePos.y);
		glm::vec2 size = { m_window->get_width(), m_window->get_height() };

		glm::vec3 intersection = glm::vec3(0.0f);

		//terrain->ray_cast(camera->generate_ray(mousePos, size), intersection);
	}

	void render() override
	{

		m_context->begin();
		scene->prepass(m_context);
		m_context->set_clear_color(0.5f, 0.7f, 1.0f, 1.0f);
		m_context->set_clear_depth(1.0f);
		m_context->begin_renderpass(nullptr, nullptr);
		m_context->set_pipeline(pipeline);
		scene->render(m_context);
		DebugDraw::render(m_context, scene->get_uniform_binding());
		m_context->end_renderpass();
		m_context->end();
	}

	void handle_input(float dt)
	{
		const float speed = 20.0f;
		const float playerSpeed = 10.0f;

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

	~TerrainExample()
	{
		terrain->destroy();
		water->destroy();
		scene->destroy();
		Device::destroy_pipeline(pipeline);
	}

private:
	Pipeline* pipeline;
	Entity* cube;
	std::shared_ptr<Scene> scene;
	Ref<Terrain> terrain;
	Ref<Water> water;
	Ref<Camera> camera;
	float mouseX = 0.0f, mouseY = 0.0f;
};

ExampleBase* CreateTerrainExampleFn()
{
	return new TerrainExample();
}