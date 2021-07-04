#include "diffuse_irradiance_generator.h"
#include "renderer/device.h"
#include "renderer/context.h"
#include "common/common.h"
#include "renderer/pipeline.h"
#include <string>

DiffuseIrradianceGenerator::DiffuseIrradianceGenerator()
{
	// Create diffuse Irraidiance map
	std::string code = load_file("spirv/diffuse_irradiance.comp.spv");
	PipelineDescription desc = {};
	ShaderDescription shader = { ShaderStage::Compute, code, static_cast<uint32_t>(code.size()) };
	desc.shaderStageCount = 1;
	desc.shaderStages = &shader;
	m_pipeline = Device::create_pipeline(desc);
}

void DiffuseIrradianceGenerator::generate(Context* context, ShaderBindings** bindings, unsigned int count, unsigned int cubemapSize)
{
	context->update_pipeline(m_pipeline, bindings, count);
	context->set_pipeline(m_pipeline);

	int uniformData[] = { int(cubemapSize), sampleCount };
	context->set_uniform(ShaderStage::Compute, 0, sizeof(uniformData), uniformData);

	context->dispatch_compute(cubemapSize / 8, cubemapSize / 8, 6);
}

void DiffuseIrradianceGenerator::destroy()
{
	Device::destroy_pipeline(m_pipeline);
}
