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

struct TextureDescription
{
	int width;
	int height;
	TextureType type;
	Format format;
	SamplerDescription* sampler;

	static TextureDescription Initialize(int width, int height)
	{
		TextureDescription desc;
		desc.width = width;
		desc.height = height;
		desc.type = TextureType::Color2D;
		desc.format = Format::R8G8B8_Unorm;
		desc.sampler = nullptr;
	}
};

class Texture
{
public:
	virtual ~Texture(){}
};