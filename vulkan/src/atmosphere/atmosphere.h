#pragma once

#include <core/base.h>
#include <core/math.h>

class Context;
class Pipeline;
class Texture;
class ShaderBindings;
class Camera;
class UniformBuffer;
class Scene;
class Mesh;

class Atmosphere
{
public:
	Atmosphere(Context* context);
	void update(Context* context, Ref<Camera> camera, glm::vec3 lightDir, float lightIntensity);
	void render(Context* context, Ref<Mesh> cube, Ref<Camera> camera, glm::vec3 lightDir, float lightIntensity);
	void destroy();

	float get_earth_radius() { return m_params.EARTH_RADIUS; }
	ShaderBindings* get_cubemap() { return m_bindings; }
private:
	struct AtmosphereParams
	{
		glm::vec3 BetaR;
		float EARTH_RADIUS;

		glm::vec3 BetaM;
		float ATMOSPHERE_RADIUS;

		float Hr;
		float Hm;
		int TRANSMITTANCE_TEXTURE_DIM;
	} m_params;

	//Uniform Buffer
	UniformBuffer* m_ubo;

	// Precomputing optical depth texture
	ShaderBindings* m_transmittanceBindings;
	Texture* m_transmittanceTexture;
	Texture* m_cubemap;
	Pipeline* m_transmittanceComputePipeline;

	// Rendering the atmosphere
    Pipeline* m_pipeline;
	ShaderBindings* m_bindings;

	Pipeline* m_cubemapPipeline;
	ShaderBindings* m_cubemapBindings;

	const uint32_t CUBEMAP_DIMS = 256;

	void precompute_transmittance(Context* context);
};