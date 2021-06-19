#pragma once

#include "graphics_enums.h"

struct SamplerDescription
{
	TextureFilter minFilter;
	TextureFilter magFilter;

	WrapMode wrapU;
	WrapMode wrapV;
	WrapMode wrapW;

	static SamplerDescription Initialize()
	{
		SamplerDescription desc = {};
		desc.minFilter = TextureFilter::Linear;
		desc.magFilter = TextureFilter::Linear;

		desc.wrapU = WrapMode::ClampToEdge;
		desc.wrapV = WrapMode::ClampToEdge;
		desc.wrapW = WrapMode::ClampToEdge;
		return desc;
	}
};

enum TextureFlag : uint32_t
{
	Sampler = 0x01,
	TransferDst = 0x02,
	TransferSrc = 0x04,
	StorageImage = 0x08
};

struct TextureDescription
{
	int width;
	int height;
	TextureType type;
	Format format;
	uint8_t flags;

	SamplerDescription* sampler;

	static TextureDescription Initialize(int width, int height)
	{
		TextureDescription desc;
		desc.width = width;
		desc.height = height;
		desc.type = TextureType::Color2D;
		desc.format = Format::R8G8B8_Unorm;
		desc.sampler = nullptr;
		return desc;
	}
};

class Texture
{
public:
	virtual ~Texture(){}

	virtual uint32_t get_height() = 0;
	virtual	uint32_t get_width() = 0;
};