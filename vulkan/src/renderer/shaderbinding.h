#pragma once

#include "graphics_enums.h"

class UniformBuffer;
class Texture;
class ShaderStorageBuffer;


class ShaderBindings
{
public:
	virtual void set_buffer(UniformBuffer* ubo, uint32_t binding) = 0;

	// Shader Storage Buffer
	virtual void set_buffer(ShaderStorageBuffer* ubo, uint32_t binding) = 0;
	
	// Image Sampler
	virtual void set_texture_sampler(Texture* texture, uint32_t binding) = 0;
	// Used for Reading and Writing for Compute Shader
	virtual void set_storage_image(Texture* texture, uint32_t binding) = 0;
	virtual ~ShaderBindings(){}
};