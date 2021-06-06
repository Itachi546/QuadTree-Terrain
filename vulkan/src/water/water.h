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

class WaterRenderer;
struct WaterProperties;

class Water
{
public:
	Water(Context* context);
	void update(Context* context, float dt);
	void render(Context* context, ShaderBindings** uniformBindings, uint32_t count);

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
	Ref<NormalMapGenerator> m_normaMapGenerator;

	Ref<WaterProperties> m_properties;
	ShaderBindings* m_fftBindings;

	glm::vec3 m_translate = glm::vec3(0.0f);
};
