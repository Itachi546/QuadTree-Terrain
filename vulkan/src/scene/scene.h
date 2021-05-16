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

class Scene
{
public:
	Scene(std::string name, Context* context);
	Entity* create_entity(std::string name);
	void destroy_entity(Entity* entity);
	void update(Context* context, float dt);
	void prepass(Context* context);
	void render(Context* context);
	void destroy();
	void set_camera(Ref<Camera> camera);

	Ref<Camera> get_camera() { return m_camera; }

	bool cast_ray(const Ray& ray, RayHit& hit);
	void get_entity_iterator(EntityIterator& begin, EntityIterator& end)
	{
		begin = m_entities.begin();
		end = m_entities.end();
	}

	ShaderBindings* get_uniform_binding() { return m_uniformBindings; }

	Entity* create_cube();
	Entity* create_plane();
	Entity* create_sphere();
private:
	struct GlobalState
	{
		glm::mat4 projection;
		glm::mat4 view;
	} m_state;

	Ref<Camera>     m_camera = nullptr;

	ShaderBindings* m_uniformBindings = nullptr;
	UniformBuffer* m_uniformBuffer = nullptr;
	UniformBuffer* m_lightUniformBuffer = nullptr;

	std::string m_name;
	std::vector<Entity*> m_entities;

	Ref<Mesh> m_cubeMesh;
	Ref<Mesh> m_planeMesh;
	Ref<Mesh> m_sphereMesh;

	Ref<ShadowCascade> m_sunLightShadowCascade;
	Ref<DirectionalLight> m_sun;
	ShaderBindings* m_lightBindings = nullptr;

	void initialize_cube_mesh(Context* context);
	void initialize_plane_mesh(Context* context);
	void initialize_sphere_mesh(Context* context);

	void _render(Context* context);
};