#pragma once

#include "graphics_enums.h"

class UniformBuffer;

struct ShaderBindingDescription
{
	uint32_t set;
};

class ShaderBindings
{
public:
	virtual void set_buffer(UniformBuffer* buffer, uint32_t binding) = 0;
	virtual void set_texture(Texture* texture, uint32_t binding) = 0;
	virtual ~ShaderBindings(){}
};