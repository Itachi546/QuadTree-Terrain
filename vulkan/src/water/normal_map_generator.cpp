#include "normal_map_generator.h"
#include "renderer/pipeline.h"
#include "renderer/context.h"
#include "renderer/device.h"
#include "renderer/texture.h"
#include "renderer/shaderbinding.h"
#include "common/common.h"

NormalMapGenerator::NormalMapGenerator(Context* context, Texture* displacementMap)
{
	std::string code = load_file("spirv/normalmap.comp.spv");
	PipelineDescription desc = {};
	ShaderDescription shader = { ShaderStage::Compute, code, static_cast<uint32_t>(code.size()) };
	desc.shaderStageCount = 1;
	desc.shaderStages = &shader;
	m_pipeline = Device::create_pipeline(desc);

	{
		uint32_t width = displacementMap->get_width();
		uint32_t height = displacementMap->get_height();
			//Normal map output texture
		TextureDescription desc = TextureDescription::Initialize(width, height);
		desc.format = Format::R32G32B32A32Float;
		desc.flags = TextureFlag::Sampler | TextureFlag::StorageImage;
		SamplerDescription samplerDesc = SamplerDescription::Initialize();
		samplerDesc.wrapU = samplerDesc.wrapV = samplerDesc.wrapW = WrapMode::Repeat;
		desc.sampler = &samplerDesc;
		m_normalTexture = Device::create_texture(desc);


		m_bindings = Device::create_shader_bindings();
		m_bindings->set_texture_sampler(displacementMap, 0);
		m_bindings->set_storage_image(m_normalTexture, 1);
		m_invResolution = 1.0f / glm::vec2(float(width), float(height));
		m_N = width;
	}
	

}

void NormalMapGenerator::destroy()
{
	Device::destroy_pipeline(m_pipeline);
	Device::destroy_shader_bindings(m_bindings);
	Device::destroy_texture(m_normalTexture);
}

void NormalMapGenerator::generate(Context* context)
{
	context->transition_layout_for_compute_read(&m_normalTexture, 1);
	context->update_pipeline(m_pipeline, &m_bindings, 1);
	context->set_pipeline(m_pipeline);

	context->set_uniform(ShaderStage::Compute, 0, sizeof(glm::vec2), &m_invResolution);

	context->dispatch_compute(m_N / 16, m_N/ 16, 1);
}
