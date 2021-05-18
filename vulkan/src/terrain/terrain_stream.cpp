#include "terrain_stream.h"
#include "perlin_noise.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

TerrainStream::TerrainStream(const char* filename)
{
	int nChannel = 0;
	uint8_t* buffer = stbi_load(filename, &m_xsize, &m_ysize, &nChannel, 1);
	ASSERT(buffer != nullptr);

	m_buffer = new float[m_xsize * m_ysize];

	float m = 1.0f / 255.0f;
	for (int y = 0; y < m_ysize; ++y)
	{
		for (int x = 0; x < m_xsize; ++x)
		{
			uint32_t index = y * m_xsize + x;
			m_buffer[index] = buffer[index] * m;
		}
	}
	stbi_image_free(buffer);
}

float fbm(siv::PerlinNoise* noise, int x, int y, const PerlinGenerator& generator)
{
	double total = 0.0;
	double normalization = 0.0;

	float a = generator.amplitude;
	float f = generator.frequency;
	for (uint32_t i = 0; i < generator.octaves; ++i)
	{
		total += a * noise->noise2D_0_1(double(x * f), double(y * f));
		normalization += a;

		a *= generator.gain;
		f *= generator.lacunarity;
	}

	total /= normalization;
	//total = total * 2.0f - 1.0f;

	return static_cast<float>(std::pow(total, generator.exponent));
}

TerrainStream::TerrainStream(const PerlinGenerator& generator) : m_xsize(generator.width), m_ysize(generator.height)
{
	m_buffer = new float[generator.width * generator.height];
	siv::PerlinNoise* noise = new siv::PerlinNoise(generator.seed);

	for (uint32_t y = 0; y < generator.height; ++y)
	{
		for (uint32_t x = 0; x < generator.height; ++x)
		{
			m_buffer[y * generator.width + x] = fbm(noise, x, y, generator);
		}
	}
}

void TerrainStream::destroy()
{
	if(m_buffer)
		delete m_buffer;
		
}
