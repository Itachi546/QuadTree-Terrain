#pragma once

#include "core/base.h"

class TerrainStream
{
public:
	// Load from heightmap
	TerrainStream(const char* filename);

	uint8_t get(int x, int y)
	{
		ASSERT(x >= 0 && x <= m_xsize);
		ASSERT(y >= 0 && y <= m_ysize);
		return m_buffer[y * m_xsize + x];
	}

	int get_width() { return m_xsize; }
	int get_height() { return m_ysize; }

	void destroy();
private:
	int m_xsize;
	int m_ysize;

	unsigned char* m_buffer = nullptr;
};
