#include "skybox.h"
#include "renderer/texture.h"
#include "common/image_loader.h"
#include "renderer/device.h"
#include "renderer/context.h"
#include "common/common.h"
#include "renderer/pipeline.h"
#include "renderer/shaderbinding.h"

Skybox::Skybox(Context* context, const char* hdriFile)
{
	create_cubemap_texture(context);
	Texture* hdri = load_hdri(context, hdriFile);
	convert_hdri_to_cubemap(context, hdri);
	Device::destroy_texture(hdri);
}

void Skybox::destroy()
{
	Device::destroy_texture(m_cubemap);
}

void Skybox::create_cubemap_texture(Context* context)
{
	TextureDescription desc = TextureDescription::Initialize(CUBEMAP_SIZE, CUBEMAP_SIZE);
	desc.flags = TextureFlag::Sampler | TextureFlag::StorageImage;
	//desc.format = Format::R16G16B16A16Float;
	desc.format = Format::R32G32B32A32Float;
	desc.type = TextureType::Cubemap;
	SamplerDescription sampler = SamplerDescription::Initialize();
	sampler.wrapU = sampler.wrapV = sampler.wrapW = WrapMode::ClampToEdge;
	desc.sampler = &sampler;
	m_cubemap = Device::create_texture(desc);
}

Texture* Skybox::load_hdri(Context* context, const char* filename)
{
	int width, height, nChannel;
	const float* data = ImageLoader::load_hdri(filename, &width, &height, &nChannel);
	ASSERT_MSG(data != nullptr, "Failed to load hdri");

	TextureDescription desc = TextureDescription::Initialize(width, height);
	desc.flags = TextureFlag::Sampler | TextureFlag::StorageImage | TextureFlag::TransferDst;
	desc.format = Format::R32G32B32A32Float;

	SamplerDescription sampler = SamplerDescription::Initialize();
	desc.sampler = &sampler;
	Texture* texture = Device::create_texture(desc);
	context->copy(texture, (void*)data, width * height * 4 * sizeof(float));
	ImageLoader::free((void*)data);
	return texture;
}

void Skybox::convert_hdri_to_cubemap(Context* context, Texture* hdri)
{
	std::string code = load_file("spirv/equirectangular_to_cubemap.comp.spv");
	PipelineDescription pipelineDesc = {};
	ShaderDescription shader = { ShaderStage::Compute, code, static_cast<uint32_t>(code.size()) };
	pipelineDesc.shaderStageCount = 1;
	pipelineDesc.shaderStages = &shader;
	Pipeline* pipeline = Device::create_pipeline(pipelineDesc);

	ShaderBindings* bindings = Device::create_shader_bindings();
	bindings->set_texture_sampler(hdri, 0);
	bindings->set_storage_image(m_cubemap, 1);

	context->begin_compute();
	context->transition_layout_for_shader_read(&hdri, 1);
	context->transition_layout_for_compute_read(&m_cubemap, 1);
	context->update_pipeline(pipeline, &bindings, 1);
	context->set_pipeline(pipeline);
	context->dispatch_compute(CUBEMAP_SIZE / 32, CUBEMAP_SIZE / 32, 6);
	context->end_compute();

	Device::destroy_pipeline(pipeline);
	Device::destroy_shader_bindings(bindings);
}
