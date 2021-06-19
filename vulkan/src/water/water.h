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
	Framebuffer* m_reflectionFB;
	Framebuffer* m_refractionFB;

	Pipeline* m_offscreenMeshPipeline;
	Pipeline* m_offscreenTerrainPipeline;
	// temp


	//Reflection and refraction texture size
	const uint32_t OFFSCREEN_WIDTH = 512;
	const uint32_t OFFSCREEN_HEIGHT = 512;

	void create_renderpass(Context* context);
	Framebuffer* create_framebuffer(Context* context);

	Pipeline* create_pipeline(Context* context, const std::string& vertexCode, const std::string& fragmentCode);

	void generate_reflection_texture(Context* context, Scene* scene);
	void generate_refraction_texture(Context* context, Scene* scene);
};
