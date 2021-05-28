#pragma once

#include "graphics_enums.h"

class UniformBuffer;
class Texture;


class ShaderBindings
{
public:
	virtual void set_buffer(UniformBuffer* ubo, uint32_t binding) = 0;
	virtual void set_texture(Texture* texture, uint32_t binding) = 0;
	virtual ~ShaderBindings(){}
};