#pragma once

struct TextureDescription;
struct FramebufferDescription
{
	uint32_t attachmentCount;
	TextureDescription* attachments;

	uint32_t width;
	uint32_t height;
};

class Texture;
class Framebuffer
{
public:
	virtual ~Framebuffer(){}

	virtual Texture* get_color_attachment(uint32_t index) = 0;
	virtual Texture* get_depth_attachment() = 0;
};