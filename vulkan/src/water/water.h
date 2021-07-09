#pragma once

#include "core/base.h"
#include "core/math.h"

class Texture;
class Pipeline;
class Context;
class ShaderBindings;
class SpectrumTexture;
class TwiddleFactors;
class ButterflyOperation;
class Inversion;
class NormalMapGenerator;
class RenderPass;
class Framebuffer;
class Camera;
class UniformBuffer;

class WaterRenderer;
struct WaterProperties;
class Scene;

class Water
{
public:
	Water(Context* context);
	void update(Context* context, float dt);
	void render(Context* context, Ref<Camera> camera, ShaderBindings** uniformBindings, uint32_t count);
	void prepass(Context* context, Scene* scene, ShaderBindings** bindings, uint32_t count);


	void set_translation(glm::vec3 translate) { m_translate = translate; }
	void destroy();

	ShaderBindings* debugBindings;
private:
	struct WaterParams
	{
		glm::vec3 waterColor;
		float maxDepth;

		glm::vec3 foamColor;
		float maxFoamDepth;

		glm::vec3 absorptionColor;
		float zN;
		float zF;

		float shoreBlendDistance;
	} m_waterParams;


	float m_timeElapsed = 0.0f;

	Ref<SpectrumTexture> m_spectrumTexture;
	Ref<TwiddleFactors> m_butterflyTexture;
	Ref<Inversion> m_inversion;
	Ref<ButterflyOperation> m_fft;
	Ref<WaterRenderer> m_renderer;
	ShaderBindings* m_rendererBindings;

	Ref<NormalMapGenerator> m_normaMapGenerator;
	Ref<WaterProperties> m_properties;
	ShaderBindings* m_fftBindings;

	glm::vec3 m_translate = glm::vec3(0.0f);

	RenderPass* m_renderPass;
	struct OffscreenPipelineInfo
	{
		Framebuffer* fb;
		Pipeline* meshPipeline;
		Pipeline* terrainPipeline;
		UniformBuffer* ubo;
		ShaderBindings* binding;
	};

	OffscreenPipelineInfo m_reflection;
	OffscreenPipelineInfo m_refraction;

	Pipeline* m_offscreenCubemapPipeline;
	UniformBuffer* m_waterUniformParams;

	//Reflection and refraction texture size
	const uint32_t OFFSCREEN_WIDTH = 512;
	const uint32_t OFFSCREEN_HEIGHT = 512;

	void create_renderpass(Context* context);
	Framebuffer* create_framebuffer(Context* context);

	Pipeline* create_pipeline(Context* context, const std::string& vertexCode, const std::string& fragmentCode);
	Pipeline* create_atmosphere_pipeline(Context* context, const std::string& vertexCode, const std::string& fragmentCode);

	void generate_offscreen_texture(Context* context, Scene* scene, OffscreenPipelineInfo* info, Ref<Camera> camera, bool reflectionPass);

	struct OffscreenUniformData
	{
		mat4 P;
		mat4 V;
		vec4 clipPlane;
		vec3 cameraPosition;
	};
};
