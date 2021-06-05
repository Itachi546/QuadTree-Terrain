#pragma once

#include "core/base.h"
#include "core/math.h"

class Texture;
class Pipeline;
class Context;
class ShaderBindings;
class SpectrumTexture;
class ButterflyTexture;
struct WaterProperties;

class Water
{
public:
	Water(Context* context);
	void render(Context* context);
	void destroy();

	ShaderBindings* debugBindings;
private:
	float m_timeElapsed = 0.0f;
	Ref<SpectrumTexture> m_spectrumTexture;
	Ref<ButterflyTexture> m_butterflyTexture;
	Ref<WaterProperties> m_properties;
};
