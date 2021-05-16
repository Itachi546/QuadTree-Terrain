#pragma once

#include "graphics_enums.h"
#include "texture.h"

struct Attachment
{
	uint32_t index;
	Format format;
	TextureType attachmentType;
};

struct RenderPassDescription
{
	uint32_t attachmentCount;
	Attachment* attachments;

	uint32_t width;
	uint32_t height;
};

class RenderPass
{
public:
	virtual ~RenderPass() {}

	virtual uint32_t get_width() = 0;
	virtual uint32_t get_height() = 0;
};