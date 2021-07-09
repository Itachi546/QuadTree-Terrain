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
	Atmosphere(Context* context, Texture* m_cubemap, UniformBuffer* viewProjection);
	void update(Context* context, Ref<Camera> camera, glm::vec3 lightDir, float lightIntensity);
	void destroy();

	float get_earth_radius() { return m_params.EARTH_RADIUS; }
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

	// Rendering to the cubemap
	Texture* m_cubemap;
	Pipeline* m_pipeline;
	ShaderBindings* m_bindings;

	void precompute_transmittance(Context* context);
};
