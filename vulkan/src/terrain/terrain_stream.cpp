#include "terrain_stream.h"
#include "perlin_noise.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <fstream>

TerrainStream::TerrainStream(const char* filename)
{
	int nChannel = 0;
	unsigned short* buffer = stbi_load_16(filename, &m_xsize, &m_ysize, &nChannel, 0);
	ASSERT(buffer != nullptr);
	m_buffer = new float[m_xsize * m_ysize];

	float m = 1.0f / 65536.0f;
	const float frequency = 10.0f;
	for (int y = 0; y < m_ysize; ++y)
	{
		for (int x = 0; x < m_xsize; ++x)
		{
			uint32_t index = y * m_xsize + x;
			m_buffer[index] = float(buffer[index]) * m;
		}
	}
	stbi_image_free(buffer);
}

/*
float ridge(float h, float offset) 
{
	h = abs(h);
	h = offset - h;
	h = h * h;
	return h;
}

float ridgedmf(vec3 p, int noctaves, float lac, float g, float off) {
	float sum = 0;
	float freq = 0.5, amp = .75;
	float prev = 0.25;
	for (int i = 0; i < noctaves; i++) {
		float n = ridge(inoise(p * freq), off);
		sum += n * amp * prev;
		prev = n;
		freq *= lac;
		amp *= g;
	}
	return (sum - 0.5) * heightScale;
}
*/

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
	total = total * 2.0f - 1.0f;

	return static_cast<float>(std::pow(total, generator.exponent));
}

TerrainStream::TerrainStream(const PerlinGenerator& generator) : m_xsize(generator.width), m_ysize(generator.height)
{
	m_buffer = new float[(generator.width + 1) * (generator.height + 1)];
	siv::PerlinNoise* noise = new siv::PerlinNoise(generator.seed);

	for (uint32_t y = 0; y < generator.height; ++y)
	{
		for (uint32_t x = 0; x < generator.height; ++x)
		{
			m_buffer[y * generator.width + x] = fbm(noise, x, y, generator);
		}
	}
}

TerrainStream::TerrainStream(float* data, uint32_t xsize, uint32_t ysize)
{
	m_xsize = xsize;
	m_ysize = ysize;
	m_buffer = data;
}

void TerrainStream::serialize(const char* filename)
{
	std::ofstream outfile(filename, std::ios::binary);
	int size[] = { m_xsize, m_ysize };
	outfile.write(reinterpret_cast<char*>(size), sizeof(int) * 2);
	outfile.write(reinterpret_cast<char*>(m_buffer), m_xsize * m_ysize * sizeof(float));
}

void TerrainStream::destroy()
{
	if(m_buffer)
		delete m_buffer;
		
}
