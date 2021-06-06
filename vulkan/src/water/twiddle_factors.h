#pragma once

#include <array>

class Pipeline;
class Context;
class ShaderBindings;
class Texture;
class ShaderStorageBuffer;

class TwiddleFactors
{

public:
	TwiddleFactors(Context* context, uint32_t N = 256);
	void create_twiddle_texture(Context* context);
	Texture* get_twiddle_texture() { return m_twiddleTexture; }
	void destroy();
private:
	void create_twiddle_texture(Context* context,Pipeline* pipeline, ShaderBindings* bindings, uint32_t N);
	Texture* m_twiddleTexture;

	// temp
	Pipeline* pipeline;
	ShaderStorageBuffer* indicesBuffer;
	ShaderBindings* bindings;
};
