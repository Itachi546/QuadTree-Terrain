#pragma once

#include "core/base.h"
#include "stb/stb_image.h"

namespace ImageLoader
{
	inline unsigned short* load_image_float(const char* filename, int* width, int* height, int* nChannel)
	{
		unsigned short* buffer = stbi_load_16(filename, width, height, nChannel, 0);
		ASSERT(buffer != nullptr);
		return buffer;
	}

	inline unsigned char* load_image(const char* filename, int* width, int* height, int* nChannel)
	{
		unsigned char* buffer = stbi_load(filename, width, height, nChannel, 0);
		ASSERT(buffer != nullptr);
		return buffer;
	}

	inline void free(void* buffer)
	{
		stbi_image_free(buffer);
	}

	inline float* load_hdri(const char* filename, int* width, int* height, int* nChannel)
	{
		stbi_set_flip_vertically_on_load(true);
		float* data = stbi_loadf(filename, width, height, nChannel, STBI_rgb_alpha);
		return data;
	}
}