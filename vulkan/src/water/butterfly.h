#pragma once

class Pipeline;
class Texture;
class Context;
class ShaderBindings;

class ButterflyOperation
{
public:
	ButterflyOperation(Context* context, unsigned int size);
	void update(Context* context, ShaderBindings* bindings, unsigned int N);

	int get_texture_index() { return m_pingpong; }
	void destroy();
private:
	Pipeline* m_pipeline;

	int m_pingpong = 0;
};