#pragma once

#include <array>

class Pipeline;
class Context;
class ShaderBindings;
class Texture;
class ShaderStorageBuffer;

class ButterflyTexture
{

public:
	ButterflyTexture(Context* context, uint32_t N = 256);
	void create_butterfly_texture(Context* context);
	Texture* get_butterfly_texture() { return m_butterflyTexture; }
	void destroy();
private:
	void create_butterfly_texture(Context* context,Pipeline* pipeline, ShaderBindings* bindings, uint32_t N);
	Texture* m_butterflyTexture;

	// temp
	Pipeline* pipeline;
	ShaderStorageBuffer* indicesBuffer;
	ShaderBindings* bindings;
};
