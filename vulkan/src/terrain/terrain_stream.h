#pragma once

#include "core/base.h"

struct PerlinGenerator
{
	float amplitude = 1.0f;
	float exponent = 1.0f;
	float frequency = 0.1f;
	float lacunarity = 2.0f;
	float gain = 0.5f;
	uint32_t octaves = 5;
	uint32_t width = 512;
	uint32_t height = 512;
	uint32_t seed = 532;
};

class TerrainStream
{
public:
	// Load from heightmap
	TerrainStream(const char* filename);
	TerrainStream(const PerlinGenerator& generator);
	TerrainStream(float* data, uint32_t xsize, uint32_t ysize);

	float get(int x, int y)
	{
		if (x < 0.0f || x >= m_xsize || y < 0.0f || y >= m_ysize)
			return 0.0f;
		//ASSERT(x >= 0 && x <= m_xsize);
		//ASSERT(y >= 0 && y <= m_ysize);
		return m_buffer[y * m_xsize + x];
	}

	void set(int x, int y, float v)
	{
		if (x < 0.0f || x > m_xsize || y < 0.0f || y > m_ysize)
			return;

		m_buffer[y * m_xsize + y] = v;
	}

	int get_width() { return m_xsize; }
	int get_height() { return m_ysize; }

	void serialize(const char* filename);
	void destroy();
private:
	int m_xsize;
	int m_ysize;
	float* m_buffer = nullptr;
};
