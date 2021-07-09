#pragma once

class Pipeline;
class ShaderBindings;
class Context;

class DiffuseIrradianceGenerator
{

public:
	DiffuseIrradianceGenerator();
	// Call between begin compute and end compute
	void generate(Context* context, ShaderBindings** bindings, unsigned int count, unsigned int irradianceCubemapSize);
	void destroy();
private:
	const int sampleCount = 32;
	Pipeline* m_pipeline;
};