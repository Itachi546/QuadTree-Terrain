#pragma once

#include <core/base.h>
#include <core/math.h>

class Context;
class Pipeline;
class Texture;
class ShaderBindings;
class Camera;
class UniformBuffer;

class Atmosphere
{
public:
	Atmosphere(Context* context);
	void render(Context* context, Ref<Camera> camera);
	void destroy();
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
	Pipeline* m_transmittanceComputePipeline;

	// Rendering the atmosphere
	Pipeline* m_pipeline;
	ShaderBindings* m_bindings;

	void precompute_transmittance(Context* context);
};