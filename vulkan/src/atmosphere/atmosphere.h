#pragma once

#include <core/base.h>
#include <core/math.h>

class DiffuseIrradianceGenerator;
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
	ShaderBindings* get_cubemap_bindings() { return m_skyboxBindings; }

	Texture* get_cubemap_texture() { return m_atmosphereCubemapInfo.texture; }
	Texture* get_irradiance_texture() { return m_diffuseIrradianceInfo.texture; }
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

	struct PipelineInfo
	{
		Pipeline* pipeline;
		ShaderBindings* bindings;
		Texture* texture;
	};

	//Uniform Buffer
	UniformBuffer* m_ubo;

	// Precomputing optical depth texture
	PipelineInfo m_transmittanceInfo;
	PipelineInfo m_atmosphereCubemapInfo;
	PipelineInfo m_diffuseIrradianceInfo;

	Ref<DiffuseIrradianceGenerator> m_DIGenerator;

	Pipeline* m_skyboxPipeline;
	ShaderBindings* m_skyboxBindings;

	const uint32_t CUBEMAP_DIMS = 256;
	const uint32_t DIFFUSE_IRRADIANCE_DIMS = 32;
	void precompute_transmittance(Context* context);
};
