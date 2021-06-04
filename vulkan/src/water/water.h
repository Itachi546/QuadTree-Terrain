#pragma once

#include "core/math.h"

class Texture;
class Pipeline;
class Context;
class ShaderBindings;

class Water
{
public:
	Water(Context* context);

	void render(Context* context);
	void destroy();

	ShaderBindings* debugBindings;
private:
	// Generates h0 and h0Tilde texture
	// Generated only once at the start of program
	Pipeline* m_spectrumPipeline;
	ShaderBindings* m_spectrumBindings;

	// Temp


	Texture* m_noiseTexture[4];
	Texture* m_h0;
	Texture* m_h0Tilde;

	// Water properties

	struct WaterProperties
	{
		//FFT Dimension
		const uint32_t dimension = 256;
		// Horizontal Dimension
		const uint32_t horizontalDimension = 1024;

		// Wind direction
		glm::vec2 m_windDirection = glm::vec2(-1.0f, 0.0f);

		// Wind Speed
		float m_windSpeed = 26;
		// Amplitude to Philip Spectrum
		float m_philipAmplitude = 1.0f;
	} m_waterProperties;
	// Time elapsed
	float m_timeElapsed = 0.0f;

	void create_spectrum_texture(Context* context);
	
};