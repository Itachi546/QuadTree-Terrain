#pragma once

class Context;
class Texture;
class Pipeline;
class ShaderBindings;

class Inversion
{
public:
	Inversion(Context* context, unsigned int N, Texture* pingpong0, Texture* pingpong1);
	void update(Context* context, unsigned int N, int pingpong);

	Texture* get_height_texture() { return m_heightTexture; }
	void destroy();
private:
	Texture* m_heightTexture;
	Pipeline* m_pipeline;

	ShaderBindings* m_bindings;
};