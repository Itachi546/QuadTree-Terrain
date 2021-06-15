#include "inversion.h"

#include "common/common.h"
#include "renderer/pipeline.h"
#include "renderer/texture.h"
#include "renderer/context.h"
#include "renderer/device.h"
#include "renderer/shaderbinding.h"

Inversion::Inversion(Context* context, unsigned int N, Texture* pingpong0, Texture* pingpong1)
{
	{
		std::string code = load_file("spirv/inversion.comp.spv");
		PipelineDescription desc = {};
		ShaderDescription shader = { ShaderStage::Compute, code, static_cast<uint32_t>(code.size()) };
		desc.shaderStageCount = 1;
		desc.shaderStages = &shader;
		m_pipeline = Device::create_pipeline(desc);
	}

	{
		TextureDescription desc = TextureDescription::Initialize(N, N);
		desc.format = Format::R32Float;
		desc.flags = TextureFlag::Sampler | TextureFlag::StorageImage;
		SamplerDescription samplerDesc = SamplerDescription::Initialize();
		samplerDesc.wrapU = samplerDesc.wrapV = samplerDesc.wrapW = WrapMode::Repeat;
		desc.sampler = &samplerDesc;
		m_heightTexture = Device::create_texture(desc);
	}

	m_bindings = Device::create_shader_bindings();
	m_bindings->set_storage_image(m_heightTexture, 0);
	m_bindings->set_storage_image(pingpong0, 1);
	m_bindings->set_storage_image(pingpong1, 2);
}

void Inversion::update(Context* context, unsigned int N, int pingpong)
{
	context->transition_layout_for_compute_read(&m_heightTexture, 1);
	context->update_pipeline(m_pipeline, &m_bindings, 1);
	context->set_pipeline(m_pipeline);


	int data[] = { pingpong, int(N) };
	context->set_uniform(ShaderStage::Compute, 0, sizeof(int) * 2, &data);
	context->dispatch_compute(N / 32, N / 32, 1);
}

void Inversion::destroy()
{
	Device::destroy_pipeline(m_pipeline);
	Device::destroy_texture(m_heightTexture);
	Device::destroy_shader_bindings(m_bindings);
}
