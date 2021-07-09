#pragma once

#include "core/base.h"

class Texture;
class Context;
class ShaderBindings;
class Pipeline;
class DiffuseIrradianceGenerator;
class Mesh;
class Camera;
class UniformBuffer;
class Atmosphere;
class Camera;
class DirectionalLight;

class Skybox
{
public:
	Skybox(Context* context, const char* hdriFile);
	Skybox(Context* context);
	void set_cubemap(Texture* cubemap) { m_cubemap = cubemap; }
	void destroy();

	void update(Context* context, Ref<Camera> camera, Ref<DirectionalLight> sun);
	void render(Context* context, Ref<Camera> camera, Ref<Mesh> cubeMesh);

	ShaderBindings* get_cubemap_bindings() { return m_bindings; }
	Texture* get_skybox_texture() { return m_cubemap; }
	Texture* get_irradiance_texture() { return m_irradiance; }
private:
	Texture* m_cubemap;
	Texture* m_irradiance;
	ShaderBindings* m_irradianceBindings;
	UniformBuffer* m_ubo;

	// render data
	Pipeline* m_pipeline;
	ShaderBindings* m_bindings;

	void initialize(Context* context);
	Texture* create_cubemap_texture(Context* context, uint32_t size);
	Texture* load_hdri(Context* context, const char* filename);
	void convert_hdri_to_cubemap(Context* context, Texture* hdri);

	void setup_render_pipeline(Context* context);

	int CUBEMAP_SIZE = 1024;
	const int IRRADIANCE_CUBMAP_SIZE = 32;

	Ref<DiffuseIrradianceGenerator> m_irradianceGenerator;
	Ref<Atmosphere> m_atmosphere;
};