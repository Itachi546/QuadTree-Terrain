#include "water.h"
#include "renderer/pipeline.h"
#include "renderer/texture.h"
#include "renderer/device.h"
#include "renderer/context.h"
#include "renderer/shaderbinding.h"
#include "common/common.h"
#include "debug/debug_draw.h"
#include <random>

void generate_random_texture(float* buffer, uint32_t width, uint32_t height)
{
	float m = 1.0f / float(RAND_MAX);
	for (uint32_t y = 0; y < height; ++y)
		for (uint32_t x = 0; x < width; ++x)
			buffer[y * width + x] = rand() * m;
}

void Water::create_spectrum_texture(Context* context)
{
	std::vector<Texture*> textures;
	for(int i = 0; i < 4; ++i)
		textures.push_back(m_noiseTexture[i]);
	textures.push_back(m_h0);
	textures.push_back(m_h0Tilde);

	context->begin_compute();
	context->transition_layout_for_compute_read(textures.data(), static_cast<uint32_t>(textures.size()));

	context->set_pipeline(m_spectrumPipeline);
	context->set_shader_bindings(&m_spectrumBindings, 1);
	context->set_uniform(ShaderStage::Compute, 0, sizeof(m_waterProperties), &m_waterProperties);
	context->dispatch_compute(8, 8, 8);
	// temp
	Texture* texArr[] = { m_h0, m_h0Tilde };
	context->transition_layout_for_shader_read(texArr, 2);
	context->end_compute();


}

Water::Water(Context* context)
{
	{
		// Initialize random texture
		uint32_t N = m_waterProperties.dimension;

		TextureDescription desc = TextureDescription::Initialize(N, N);
		desc.format = Format::R32Float;
		desc.flags = TextureFlag::Sampler | TextureFlag::TransferDst | TextureFlag::StorageImage;
		SamplerDescription samplerDesc = SamplerDescription::Initialize();
		desc.sampler = &samplerDesc;

		srand(0xF325);
		float* buffer = new float[N * N];
		for (int i = 0; i < 4; ++i)
		{
			m_noiseTexture[i] = Device::create_texture(desc);
			generate_random_texture(buffer, N, N);
			context->copy(m_noiseTexture[i], buffer, sizeof(float) * N * N);
		}

		// h0 and h0Tilde texture
		desc.format = Format::R32G32Float;
		desc.flags = TextureFlag::StorageImage | TextureFlag::Sampler;

		m_h0 = Device::create_texture(desc);
		m_h0Tilde = Device::create_texture(desc);
	}

	{
		std::string code = load_file("spirv/philipSpectrum.comp.spv");
		PipelineDescription desc = {};
		ShaderDescription shader = {ShaderStage::Compute, code, static_cast<uint32_t>(code.size())};
		desc.shaderStageCount = 1;
		desc.shaderStages = &shader;
		m_spectrumPipeline = Device::create_pipeline(desc);

		m_spectrumBindings = Device::create_shader_bindings();
		for (int i = 0; i < 4; ++i)
			m_spectrumBindings->set_storage_image(m_noiseTexture[i], i);
		m_spectrumBindings->set_storage_image(m_h0, 4);
		m_spectrumBindings->set_storage_image(m_h0Tilde, 5);
	}
	debugBindings = Device::create_shader_bindings();
	debugBindings->set_texture_sampler(m_h0Tilde, 0);
	create_spectrum_texture(context);
}

void Water::render(Context* context)
{
}

void Water::destroy()
{
	Device::destroy_pipeline(m_spectrumPipeline);
	for (int i = 0; i < 4; ++i)
		Device::destroy_texture(m_noiseTexture[i]);

	Device::destroy_texture(m_h0);
	Device::destroy_texture(m_h0Tilde);
}

