#include "butterfly_texture.h"
#include "renderer/pipeline.h"
#include "renderer/texture.h"
#include "renderer/buffer.h"
#include "renderer/context.h"
#include "common/common.h"
#include "renderer/device.h"
#include "renderer/shaderbinding.h"

ButterflyTexture::ButterflyTexture(Context* context, uint32_t N)
{
	// Setup pipeline
	{
		std::string code = load_file("spirv/butterfly.comp.spv");
		PipelineDescription desc = {};
		ShaderDescription shader = { ShaderStage::Compute, code, static_cast<uint32_t>(code.size()) };
		desc.shaderStageCount = 1;
		desc.shaderStages = &shader;
		pipeline = Device::create_pipeline(desc);
	}

	// Setup texture with horizontal dimension of log2(N), one for each stage
	// vertical dimension of N (log2(N) * N)
	{
		TextureDescription desc = TextureDescription::Initialize(static_cast<int>(std::log2(N)), N);
		desc.format = Format::R32G32B32A32Float;
		desc.flags = TextureFlag::Sampler | TextureFlag::StorageImage;
		SamplerDescription samplerDesc = SamplerDescription::Initialize();
		samplerDesc.minFilter = samplerDesc.magFilter = TextureFilter::Nearest;
		desc.sampler = &samplerDesc;

		m_butterflyTexture = Device::create_texture(desc);
	}


	// Setup the uniform buffer for butterfly indices
	std::array<int, 256> indices = {
		0,128,64,192,32,160,96,224,16,144,80,208,48,176,

		112,240,8,136,72,200,40,168,104,232,24,152,88,216,

		56,184,120,248,4,132,68,196,36,164,100,228,20,148,

		84,212,52,180,116,244,12,140,76,204,44,172,108,236,

		28,156,92,220,60,188,124,252,2,130,66,194,34,162,98,

		226,18,146,82,210,50,178,114,242,10,138,74,202,42,170,

		106,234,26,154,90,218,58,186,122,250,6,134,70,198,38,166,

		102,230,22,150,86,214,54,182,118,246,14,142,78,206,46,174,

		110,238,30,158,94,222,62,190,126,254,1,129,65,193,33,

		161,97,225,17,145,81,209,49,177,113,241,9,137,73,201,41,

		169,105,233,25,153,89,217,57,185,121,249,5,133,69,197,37,

		165,101,229,21,149,85,213,53,181,117,245,13,141,77,205,

		45,173,109,237,29,157,93,221,61,189,125,253,3,131,67,195,

		35,163,99,227,19,147,83,211,51,179,115,243,11,139,75,203,

		43,171,107,235,27,155,91,219,59,187,123,251,7,135,71,199,

		39,167,103,231,23,151,87,215,55,183,119,247,15,143,79,207,

		47,175,111,239,31,159,95,223,63,191,127,255,
	};
	{
		uint32_t sizeInByte = static_cast<uint32_t>(indices.size()) * sizeof(int);
		indicesBuffer = Device::create_shader_storage_buffer(BufferUsageHint::StaticRead, sizeInByte);
		context->copy(indicesBuffer, indices.data(), 0, sizeInByte);
	}

	bindings = Device::create_shader_bindings();
	bindings->set_storage_image(m_butterflyTexture, 0);
	bindings->set_buffer(indicesBuffer, 1);

	//create_butterfly_texture(context, pipeline, bindings, N);

	//Device::destroy_buffer(indicesBuffer);
	//Device::destroy_shader_bindings(bindings);
	//Device::destroy_pipeline(pipeline);
}

void ButterflyTexture::create_butterfly_texture(Context* context)
{
	create_butterfly_texture(context, pipeline, bindings, 256);
}

void ButterflyTexture::destroy()
{
	Device::destroy_buffer(indicesBuffer);
	Device::destroy_shader_bindings(bindings);
	Device::destroy_pipeline(pipeline);
	Device::destroy_texture(m_butterflyTexture);
}

void ButterflyTexture::create_butterfly_texture(Context* context, Pipeline* pipeline, ShaderBindings* bindings, uint32_t N)
{
	
	context->begin_compute();
	context->transition_layout_for_compute_read(&m_butterflyTexture, 1);
	context->set_pipeline(pipeline);
	context->set_shader_bindings(&bindings, 1);
	context->dispatch_compute(8, 16, 1);

	// temp
	context->transition_layout_for_shader_read(&m_butterflyTexture, 1);
	context->end_compute();
}
