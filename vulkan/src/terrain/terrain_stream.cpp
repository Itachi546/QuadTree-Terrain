#include "terrain_stream.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

TerrainStream::TerrainStream(const char* filename)
{
	int nChannel = 0;
	m_buffer = stbi_load(filename, &m_xsize, &m_ysize, &nChannel, 1);

	ASSERT(m_buffer != nullptr);
}

void TerrainStream::destroy()
{
	stbi_image_free(m_buffer);
}
