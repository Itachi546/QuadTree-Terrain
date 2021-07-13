#include "scene.h"
#include "core/math.h"
#include "renderer/renderer.h"
#include "gpu_memory.h"
#include "camera.h"
#include "light/cascaded_shadow.h"
#include "light/directional_light.h"
#include "terrain/terrain.h"
#include "water/water.h"
#include "core/frustum.h"
#include "common/common.h"
#include "utils/skybox.h"
#include "imgui/imgui.h"

struct LightData
{
	glm::vec3 lightDirection;
	float intensity;
	glm::vec3 lightColor;
	float castShadow;
};

Scene::Scene(std::string name, Context* context) : m_name(name)
{
	m_uniformBuffer = Device::create_uniformbuffer(BufferUsageHint::DynamicDraw, sizeof(GlobalState));

	m_state.projection = glm::mat4(1.0f);
	m_state.view = glm::mat4(1.0f);

	initialize_sphere_mesh(context);
	initialize_plane_mesh(context);
	initialize_cube_mesh(context);

	glm::vec3 lightDir = glm::normalize(glm::vec3(0.0f, 1.0f, -1.0f));
	m_sun = CreateRef<DirectionalLight>(lightDir);
	m_sun->set_light_color(glm::vec3(1.28f, 1.20f, 0.99f));
	m_sun->set_cast_shadow(true);
	m_sun->set_intensity(1.978f);

	m_sunLightShadowCascade = CreateRef<ShadowCascade>(lightDir);

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
	pipelineDesc.renderPass = context->get_global_renderpass();
	pipelineDesc.rasterizationState.depthTestFunction = CompareOp::LessOrEqual;
	pipelineDesc.rasterizationState.enableDepthTest = true;
	pipelineDesc.rasterizationState.faceCulling = FaceCulling::Back;
	pipelineDesc.rasterizationState.topology = Topology::Triangle;
	m_pipeline = Device::create_pipeline(pipelineDesc);

	m_lightUniformBuffer = Device::create_uniformbuffer(BufferUsageHint::DynamicDraw, sizeof(LightData));
	m_uniformBindings = Device::create_shader_bindings();
	m_uniformBindings->set_buffer(m_uniformBuffer, 0);

	m_skybox = CreateRef<Skybox>(context);
	m_lightBindings = Device::create_shader_bindings();
	m_lightBindings->set_buffer(m_lightUniformBuffer, 1);
	m_lightBindings->set_texture_sampler(m_skybox->get_skybox_texture(), 2);
	m_lightBindings->set_texture_sampler(m_skybox->get_irradiance_texture(), 3);
}

Entity* Scene::create_entity(std::string name)
{
	Ref<Transform> transform = CreateRef<Transform>();
	Entity* entity = new Entity(name, transform);
	entity->material = CreateRef<Material>();

	m_entities.push_back(entity);
	return entity;
}

void Scene::destroy_entity(Entity* entity)
{
	auto it = std::find(m_entities.begin(), m_entities.end(), entity);
	if (it != m_entities.end())
		m_entities.erase(it);

	delete entity;
}

void Scene::update(Context* context, float dt)
{
	m_elapsedTime += dt;
	m_camera->update(dt);

	if (m_sun->cast_shadow())
		m_sunLightShadowCascade->update(m_camera);

	m_state.projection = m_camera->get_projection();
	m_state.view = m_camera->get_view();
	m_state.cameraPosition = m_camera->get_position();

	context->copy(m_uniformBuffer, &m_state, 0, sizeof(GlobalState));

	LightData sun = { m_sun->get_direction(), m_sun->get_intensity(), m_sun->get_light_color(), float(m_sun->cast_shadow()) };
	context->copy(m_lightUniformBuffer, &sun, 0, sizeof(LightData));
	if(m_skybox)
		m_skybox->update(context, m_camera, m_sun);
}

void Scene::prepass(Context* context)
{
	m_sunLightShadowCascade->render(context, this, m_sun->cast_shadow());
	if (m_water)
	{
		m_water->prepass(context, this, &m_lightBindings, 1);
	}
}

void Scene::render(Context* context)
{
	render_ui();
	ShaderBindings* bindings[3];
	bindings[0] = m_uniformBindings;
	bindings[1] = m_lightBindings;
	bindings[2] = m_sunLightShadowCascade->get_depth_bindings();
	uint32_t bindingCount = ARRAYSIZE(bindings);
	context->update_pipeline(m_pipeline, bindings, bindingCount);
	context->set_pipeline(m_pipeline);
	render_entities(context, m_camera, 0);

	if (m_terrain)
		m_terrain->render(context, m_camera, bindings, bindingCount, m_elapsedTime);
	if (m_water)
		m_water->render(context, m_camera, bindings, 2);

	if(m_skybox)
		m_skybox->render(context, m_camera, m_cubeMesh);

	DebugDraw::render(context, m_uniformBindings);
}

void Scene::render_entities(Context* context, Ref<Camera> camera, uint32_t modelMatrixOffset)
{
	Ref<Frustum> frustum = camera->get_frustum();
	for (auto& entity : m_entities)
	{
		Ref<Transform> transform = entity->transform;
		Ref<Mesh> mesh = entity->mesh;
		Ref<Material> mat = entity->material;

		BoundingBox box = mesh->boundingBox;
		if (frustum->intersect_box(BoundingBox{ transform->position + box.min * transform->scale, transform->position + box.max * transform->scale }))
		{
			glm::mat4 model = transform->get_mat4();
			if (m_showBoundingBox)
			{
				glm::vec3 scale = (mesh->boundingBox.max - mesh->boundingBox.min) * transform->scale * 0.5f;
				DebugDraw::draw_box(glm::translate(glm::mat4(1.0f), transform->position) * glm::scale(glm::mat4(1.0f), scale));
			}

			context->set_buffer(mesh->vb->buffer, mesh->vb->offset);
			context->set_buffer(mesh->ib->buffer, mesh->ib->offset);
			context->set_uniform(ShaderStage::Vertex, modelMatrixOffset, sizeof(mat4), &model[0][0]);
			context->set_uniform(ShaderStage::Fragment, modelMatrixOffset + sizeof(mat4), sizeof(Material), mat.get());

			context->draw_indexed(mesh->get_indices_count());
		}
	}

}


void Scene::destroy()
{
	m_sunLightShadowCascade->destroy();
	if(m_skybox)
		m_skybox->destroy();
	
	for (std::size_t i = 0; i < m_entities.size(); ++i)
		delete m_entities[i];
	m_entities.clear();
	Device::destroy_buffer(m_uniformBuffer);
	Device::destroy_buffer(m_lightUniformBuffer);
	Device::destroy_pipeline(m_pipeline);
}

void Scene::set_camera(Ref<Camera> camera)
{
	m_camera = camera;
	m_sunLightShadowCascade->update(camera);
}

bool Scene::cast_ray(const Ray& ray, RayHit& hit)
{
	float min_dist = FLT_MAX;
	bool intersect = false;
	Entity* intersectedEntity = nullptr;
	for (auto& entity : m_entities)
	{
		Ref<Transform> transform = entity->transform;
		BoundingBox box = entity->mesh->boundingBox;
		float t = 0.0f;
		if (ray.intersect_box(transform->position + box.min * transform->scale, transform->position + box.max * transform->scale, t))
		{
			if (t < min_dist)
			{
				intersect = true;
				intersectedEntity = entity;
				min_dist = t;
			}
		}
	}
	hit.entity = intersectedEntity;
	hit.t = min_dist;
	return intersect;
}

void Scene::set_skybox(Ref<Skybox> skybox)
{
	m_skybox->destroy();
	m_skybox = skybox;
	m_lightBindings->set_texture_sampler(m_skybox->get_skybox_texture(), 2);
	m_lightBindings->set_texture_sampler(m_skybox->get_irradiance_texture(), 3);
}

Entity* Scene::create_cube()
{
	Entity* entity = create_entity("cube");
	entity->mesh = m_cubeMesh;
	return entity;
}

Entity* Scene::create_plane()
{
	Entity* entity = create_entity("plane");
	entity->mesh = m_planeMesh;
	return entity;
}

Entity* Scene::create_sphere()
{
	Entity* entity = create_entity("sphere");
	entity->mesh = m_sphereMesh;
	return entity;
}

void Scene::initialize_cube_mesh(Context* context)
{
	m_cubeMesh = CreateRef<Mesh>();

	m_cubeMesh->vertices = {
		Vertex{vec3(-1.0f, +1.0f, +1.0f), vec3(+0.0f, +1.0f, +0.0f)},
		Vertex{vec3(+1.0f, +1.0f, +1.0f), vec3(+0.0f, +1.0f, +0.0f)},
		Vertex{vec3(+1.0f, +1.0f, -1.0f), vec3(+0.0f, +1.0f, +0.0f)},

		Vertex{vec3(-1.0f, +1.0f, -1.0f), vec3(+0.0f, +1.0f, +0.0f)},
		Vertex{vec3(-1.0f, +1.0f, -1.0f), vec3(+0.0f, +0.0f, -1.0f)},
		Vertex{vec3(+1.0f, +1.0f, -1.0f), vec3(+0.0f, +0.0f, -1.0f)},

		Vertex{vec3(+1.0f, -1.0f, -1.0f), vec3(+0.0f, +0.0f, -1.0f)},
		Vertex{vec3(-1.0f, -1.0f, -1.0f), vec3(+0.0f, +0.0f, -1.0f)},
		Vertex{vec3(+1.0f, +1.0f, -1.0f), vec3(+1.0f, +0.0f, +0.0f)},

		Vertex{vec3(+1.0f, +1.0f, +1.0f), vec3(+1.0f, +0.0f, +0.0f)},
		Vertex{vec3(+1.0f, -1.0f, +1.0f), vec3(+1.0f, +0.0f, +0.0f)},
		Vertex{vec3(+1.0f, -1.0f, -1.0f), vec3(+1.0f, +0.0f, +0.0f)},

		Vertex{vec3(-1.0f, +1.0f, +1.0f), vec3(-1.0f, +0.0f, +0.0f)},
		Vertex{vec3(-1.0f, +1.0f, -1.0f), vec3(-1.0f, +0.0f, +0.0f)},
		Vertex{vec3(-1.0f, -1.0f, -1.0f), vec3(-1.0f, +0.0f, +0.0f)},

		Vertex{vec3(-1.0f, -1.0f, +1.0f), vec3(-1.0f, +0.0f, +0.0f)},
		Vertex{vec3(+1.0f, +1.0f, +1.0f), vec3(+0.0f, +0.0f, +1.0f)},
		Vertex{vec3(-1.0f, +1.0f, +1.0f), vec3(+0.0f, +0.0f, +1.0f)},

		Vertex{vec3(-1.0f, -1.0f, +1.0f), vec3(+0.0f, +0.0f, +1.0f)},
		Vertex{vec3(+1.0f, -1.0f, +1.0f), vec3(+0.0f, +0.0f, +1.0f)},
		Vertex{vec3(+1.0f, -1.0f, -1.0f), vec3(+0.0f, -1.0f, +0.0f)},

		Vertex{vec3(-1.0f, -1.0f, -1.0f), vec3(+0.0f, -1.0f, +0.0f)},
		Vertex{vec3(-1.0f, -1.0f, +1.0f), vec3(+0.0f, -1.0f, +0.0f)},
		Vertex{vec3(+1.0f, -1.0f, +1.0f), vec3(+0.0f, -1.0f, +0.0f)},
	};

	m_cubeMesh->indices = {
		0,   1,  2,  0,  2,  3, // Top
		4,   5,  6,  4,  6,  7, // Front
		8,   9, 10,  8, 10, 11, // Right
		12, 13, 14, 12, 14, 15, // Left
		16, 17, 18, 16, 18, 19, // Back
		20, 22, 21, 20, 23, 22, // Bottom
	};

	m_cubeMesh->boundingBox.min = glm::vec3(-1.0f);
	m_cubeMesh->boundingBox.max = glm::vec3(1.0f);

	m_cubeMesh->finalize(context);
}

void Scene::initialize_plane_mesh(Context* context)
{
	m_planeMesh = CreateRef<Mesh>();

	m_planeMesh->vertices = {
		Vertex{vec3(-1.0f,  0.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f)},
		Vertex{vec3(-1.0f,  0.0f,  1.0f), vec3(0.0f, 1.0f, 0.0f)},
		Vertex{vec3( 1.0f,  0.0f,  1.0f), vec3(0.0f, 1.0f, 0.0f)},
		Vertex{vec3( 1.0f,  0.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f)},
	};

	m_planeMesh->indices = 
	{
		0, 1, 3,
		1, 2, 3
	};

	m_planeMesh->boundingBox.min = glm::vec3(-1.0f, -0.05f, -1.0f);
	m_planeMesh->boundingBox.max = glm::vec3( 1.0f,  0.05f,  1.0f);
	m_planeMesh->finalize(context);
}

void Scene::initialize_sphere_mesh(Context* context)
{
	m_sphereMesh = CreateRef<Mesh>();

	const int nSector = 32;
	const int nStack = 32;
	const float radius = 1.0f;
	std::vector<Vertex> vertices;

	constexpr float PI2 = static_cast<float>(2.0 * PI);
	const float sectorStep = float(PI2) / float(nSector);
	const float stackStep  = float(PI) / float(nStack);

	for (int stack = 0; stack <= nStack; ++stack)
	{
		float stackAngle = -float(PI_2) + stackStep * stack;
		float xy = radius * std::cos(stackAngle);
		float z = radius * std::sin(stackAngle);

		for (int sector = 0; sector <= nSector; ++sector)
		{
			float sectorAngle = sectorStep * sector;

			Vertex vertex;
			vertex.position = glm::vec3{ xy * std::cos(sectorAngle),
				z, xy * std::sin(sectorAngle)};

			vertex.normal = vertex.position / radius;
			vertices.push_back(vertex);
		}
	}

	std::vector<uint32_t> indices;

	for (int i = 0; i < nStack; ++i)
	{
		int k1 = i * (nSector + 1);
		int k2 = k1 + nSector + 1;

		for (int j = 0; j < nSector; ++j, ++k1, ++k2)
		{
			if (i != 0)
			{
				indices.push_back(k1);
				indices.push_back(k2);
				indices.push_back(k1 + 1);
			}

			if (i != (nStack - 1))
			{
				indices.push_back(k1 + 1);
				indices.push_back(k2);
				indices.push_back(k2 + 1);
			}
		}
	}

	m_sphereMesh->vertices = vertices;
	m_sphereMesh->indices = indices;

	m_sphereMesh->boundingBox.min = glm::vec3(-1.0f);
	m_sphereMesh->boundingBox.max = glm::vec3( 1.0f);
	m_sphereMesh->finalize(context);
}

void Scene::render_ui()
{
	if (ImGui::CollapsingHeader(m_name.c_str()))
	{
		ImGui::Text("Total Entities: %d", m_entities.size());
		ImGui::Checkbox("Show BoundingBox", &m_showBoundingBox);
		if (ImGui::SliderFloat3("Sun Direction", &m_sun->m_direction[0], -1.0f, 1.0f))
			m_sunLightShadowCascade->set_light_direction(m_sun->get_direction());

		ImGui::SliderFloat("Sun Intensity", &m_sun->m_intensity, 1.0f, 10.0f);
		ImGui::ColorPicker3("Sun Color", &m_sun->m_lightColor[0]);
		ImGui::Checkbox("Cast Shadow ", &m_sun->m_castShadow);
	}
}

