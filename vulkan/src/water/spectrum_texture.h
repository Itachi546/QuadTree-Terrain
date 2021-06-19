#pragma once

// Generate Philip spectrum texture for Ocean Rendering
// h0(k) = (1 / sqrt(2)) * (E0 + iE1) * sqrt(Ph(k))
// where E0 and E1 are gaussian random variable with mean = 0, S.D = 1
// Ph(k) is a Philips spectrum given by
// Ph(k) = A (exp(-1 / (k2 * l2)) / (k2 * k2)) * dot(k, windDirection)
// k2 = dot(k, k)

#include "core/base.h"
#include "core/math.h"

class Texture;
class Pipeline;
class ShaderBindings;
class Context;

struct WaterProperties
{
	//FFT Dimension
	uint32_t dimension;
	// Horizontal Dimension
	uint32_t horizontalDimension;

	// Wind direction
	glm::vec2 windDirection;

	// Wind Speed
	float windSpeed;
	// Amplitude to Philip Spectrum
	float philipAmplitude;
};


class SpectrumTexture
{
public:
	SpectrumTexture(Context* context, Ref<WaterProperties> props);
	void create_spectrum_texture(Context* context, Ref<WaterProperties> props);
	void create_hdt_texture(Context* context, float elapsedTime, uint32_t L);
	void destroy();

	Texture* get_h0_texture() { return m_h0TildeK; };
	Texture* get_h0_tilde_texture() { return m_h0TildeMinusK; }
	Texture* get_pingpoing_texture0() { return m_pingpongTexture0; }
	Texture* get_pingpoing_texture1() { return m_pingpongTexture1; }
private:
	void destroy_intermediate_data();
	// Generates h0 and h0Tilde texture
	// Generated only once at the start of program
	Pipeline* m_spectrumPipeline;
	ShaderBindings* m_spectrumBindings;

	Pipeline* m_hdtPipeline;
	ShaderBindings* m_hdtBindings;

	Texture* m_noiseTexture[4];
	Texture* m_h0TildeK;
	Texture* m_h0TildeMinusK;

	Texture* m_pingpongTexture0;
	Texture* m_pingpongTexture1;

	uint32_t m_N;
	// Water properties
};
