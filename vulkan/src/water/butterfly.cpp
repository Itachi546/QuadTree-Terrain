#include "butterfly.h"
#include "core/math.h"

#include "common/common.h"
#include "renderer/context.h"
#include "renderer/pipeline.h"
#include "renderer/device.h"
#include "renderer/texture.h"
#include "renderer/shaderbinding.h"


ButterflyOperation::ButterflyOperation(Context* context, unsigned int size)
{
	{
		std::string code = load_file("spirv/butterflyOperation.comp.spv");
		PipelineDescription desc = {};
		ShaderDescription shader = { ShaderStage::Compute, code, static_cast<uint32_t>(code.size()) };
		desc.shaderStageCount = 1;
		desc.shaderStages = &shader;
		m_pipeline = Device::create_pipeline(desc);
	}
}

void ButterflyOperation::update(Context* context, ShaderBindings* bindings, unsigned int N)
{
	int stages = static_cast<int>(std::log2(N));
	context->set_pipeline(m_pipeline);
	context->set_shader_bindings(&bindings, 1);

	for (int stage = 0; stage < stages; ++stage)
	{
		glm::ivec3 data = glm::ivec3(stage, m_pingpong, 0);
		context->set_uniform(ShaderStage::Compute, 0, sizeof(int) * 3, &data);
		context->dispatch_compute(N/16, N/16, 1);
		m_pingpong = (m_pingpong + 1) % 2;
	}

	for (int stage = 0; stage < stages; ++stage)
	{
		glm::ivec3 data = glm::ivec3(stage, m_pingpong, 1);
		context->set_uniform(ShaderStage::Compute, 0, sizeof(int) * 3, &data);
		context->dispatch_compute(N / 16, N / 16, 1);
		m_pingpong = (m_pingpong + 1) % 2;
	}

}

void ButterflyOperation::destroy()
{
	Device::destroy_pipeline(m_pipeline);
}

