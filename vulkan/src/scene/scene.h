#pragma once

#include "entity.h"
#include "core/ray.h"

class GraphicsWindow;
class Context;
class ShaderBindings;
class Pipeline;
class RenderPass;
class Framebuffer;
class UniformBuffer;
class Camera;
class ShadowCascade;
class DirectionalLight;
class Terrain;
class Water;
class Skybox;
class Atmosphere;

class Scene
{
public:
	Scene(std::string name, Context* context);
	Entity* create_entity(std::string name);
	void destroy_entity(Entity* entity);

	void prepass(Context* context);
	void update(Context* context, float dt);
	void render(Context* context);
	void destroy();

	// modelMatrixOffset is offset to the model matrix in push constant
	void render_entities(Context* context, Ref<Camera> camera, uint32_t modelMatrixOffset);

	void set_camera(Ref<Camera> camera);
	void show_bounding_box(bool state) { m_showBoundingBox = state; }

	Ref<Camera> get_camera() { return m_camera; }

	bool cast_ray(const Ray& ray, RayHit& hit);
	void get_entity_iterator(EntityIterator& begin, EntityIterator& end)
	{
		begin = m_entities.begin();
		end = m_entities.end();
	}

	void set_terrain(Ref<Terrain> terrain) { m_terrain = terrain; }
	Ref<Terrain> get_terrain() { return m_terrain; }

	void set_water(Ref<Water> water) { m_water = water; }

	void set_skybox(Ref<Skybox> skybox);
	Ref<Skybox> get_skybox() { return m_skybox; }

	ShaderBindings* get_uniform_binding() { return m_uniformBindings; }
	Ref<DirectionalLight> get_directional_light() { return m_sun; }

	Entity* create_cube();
	Entity* create_plane();
	Entity* create_sphere();

	Ref<Mesh> get_cube_mesh() { return m_cubeMesh; }
private:
	struct GlobalState
	{
		glm::mat4 projection;
		glm::mat4 view;
		glm::vec3 cameraPosition;
	} m_state;

	Ref<Camera>     m_camera = nullptr;

	ShaderBindings* m_uniformBindings = nullptr;
	UniformBuffer* m_uniformBuffer = nullptr;
	UniformBuffer* m_lightUniformBuffer = nullptr;
	Pipeline* m_pipeline;

	std::string m_name;
	std::vector<Entity*> m_entities;

	Ref<Mesh> m_cubeMesh;
	Ref<Mesh> m_planeMesh;
	Ref<Mesh> m_sphereMesh;

	Ref<Terrain> m_terrain;
	Ref<Water> m_water;

	Ref<ShadowCascade> m_sunLightShadowCascade;
	Ref<DirectionalLight> m_sun;
	ShaderBindings* m_lightBindings = nullptr;
	

	Ref<Skybox> m_skybox;
	// Debug
	bool m_showBoundingBox = false;

	void initialize_cube_mesh(Context* context);
	void initialize_plane_mesh(Context* context);
	void initialize_sphere_mesh(Context* context);

	void render_ui();


};