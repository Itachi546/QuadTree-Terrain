#include "scene.h"
#include "core/math.h"
#include "renderer/renderer.h"
#include "gpu_memory.h"
#include "camera.h"
#include "light/cascaded_shadow.h"
#include "light/directional_light.h"

struct LightData
{
	glm::vec3 lightDirection;
	float intensity;
	glm::vec3 lightColor;
	bool castShadow;
};

Scene::Scene(std::string name, Context* context)
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

	m_lightUniformBuffer = Device::create_uniformbuffer(BufferUsageHint::DynamicDraw, sizeof(LightData));
	m_uniformBindings = Device::create_shader_bindings();
	m_uniformBindings->set_buffer(m_uniformBuffer, 0);

	m_lightBindings = Device::create_shader_bindings();
	m_lightBindings->set_buffer(m_lightUniformBuffer, 1);

	m_sunLightShadowCascade = CreateRef<ShadowCascade>(lightDir);
}

Entity* Scene::create_entity(std::string name)
{
	Ref<Transform> transform = CreateRef<Transform>();
	Entity* entity = new Entity(name, transform);
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
	m_camera->update(dt);
	m_sunLightShadowCascade->update(m_camera);
	m_state.projection = m_camera->get_projection();
	m_state.view = m_camera->get_view();
	context->copy(m_uniformBuffer, &m_state, 0, sizeof(GlobalState));

	LightData sun = { m_sun->get_direction(), m_sun->get_intensity(), m_sun->get_light_color(), m_sun->cast_shadow() };
	context->copy(m_lightUniformBuffer, &sun, 0, sizeof(LightData));
}

void Scene::prepass(Context* context)
{
	m_sunLightShadowCascade->render(context, this);
}

void Scene::render(Context* context)
{
	ShaderBindings* bindings[3];
	bindings[0] = m_uniformBindings;
	bindings[1] = m_lightBindings;
	bindings[2] = m_sunLightShadowCascade->get_depth_bindings();
	context->set_shader_bindings(bindings, ARRAYSIZE(bindings));

	_render(context);
}

void Scene::_render(Context* context)
{
	glm::mat4 model = glm::mat4(1.0f);
	for (auto& entity : m_entities)
	{
		glm::mat4 model = entity->transform->get_mat4();
		Ref<Mesh> mesh = entity->mesh;
		context->set_buffer(mesh->vb->buffer, mesh->vb->offset);
		context->set_buffer(mesh->ib->buffer, mesh->ib->offset);
		context->set_uniform(ShaderStage::Vertex, 0, sizeof(mat4), &model[0][0]);
		context->draw_indexed(mesh->get_indices_count());
	}
}


void Scene::destroy()
{
	m_sunLightShadowCascade->destroy();
	for (std::size_t i = 0; i < m_entities.size(); ++i)
		delete m_entities[i];
	m_entities.clear();
	Device::destroy_buffer(m_uniformBuffer);
	Device::destroy_buffer(m_lightUniformBuffer);
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
	m_sphereMesh->finalize(context);
}

