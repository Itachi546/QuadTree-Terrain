#include "atmosphere.h"
#include "renderer/context.h"
#include "renderer/pipeline.h"
#include "renderer/texture.h"
#include "renderer/shaderbinding.h"
#include "renderer/device.h"
#include "common/common.h"
#include "debug/debug_draw.h"
#include "scene/camera.h"
#include "renderer/buffer.h"

Atmosphere::Atmosphere(Context* context)
{
	m_params.EARTH_RADIUS = 6.360e6f;
	m_params.ATMOSPHERE_RADIUS = 6.42e6f;

	m_params.Hr = 7.994e3f;
	m_params.Hm = 1.2e3f;

	m_params.BetaR = glm::vec3(5.8e-6f, 13.5e-6f, 33.1e-6f);
	m_params.BetaM = glm::vec3(21e-6f);
	m_params.TRANSMITTANCE_TEXTURE_DIM = 512;

	m_ubo = Device::create_uniformbuffer(BufferUsageHint::StaticDraw, sizeof(m_params));
	context->copy(m_ubo, &m_params, 0, sizeof(m_params));

  {
		std::string code = load_file("spirv/precompute_transmittance.comp.spv");
		PipelineDescription desc = {};
		ShaderDescription shader = { ShaderStage::Compute, code, static_cast<uint32_t>(code.size()) };
		desc.shaderStageCount = 1;
		desc.shaderStages = &shader;
		m_transmittanceComputePipeline = Device::create_pipeline(desc);
  }

  {
	  TextureDescription desc = TextureDescription::Initialize(m_params.TRANSMITTANCE_TEXTURE_DIM, m_params.TRANSMITTANCE_TEXTURE_DIM);
	  desc.flags = TextureFlag::Sampler | TextureFlag::StorageImage;
	  desc.format = Format::R16G16B16A16Float;
	  desc.type = TextureType::Color2D;
	  SamplerDescription sampler = SamplerDescription::Initialize();
	  sampler.wrapU = sampler.wrapV = sampler.wrapW = WrapMode::ClampToEdge;
	  desc.sampler = &sampler;
	  m_transmittanceTexture = Device::create_texture(desc);
  }

  {
	  m_transmittanceBindings = Device::create_shader_bindings();
	  m_transmittanceBindings->set_storage_image(m_transmittanceTexture, 0);
	  m_transmittanceBindings->set_buffer(m_ubo, 1);
	  precompute_transmittance(context);
  }

  {
	  std::string vertexCode = load_file("spirv/atmosphere.vert.spv");
	  ASSERT(vertexCode.size() % 4 == 0);
	  std::string fragmentCode = load_file("spirv/atmosphere.frag.spv");
	  ASSERT(fragmentCode.size() % 4 == 0);

	  PipelineDescription pipelineDesc = {};
	  ShaderDescription shaderDescription[2] = {};
	  shaderDescription[0].shaderStage = ShaderStage::Vertex;
	  shaderDescription[0].code = vertexCode;
	  shaderDescription[0].sizeInByte = static_cast<uint32_t>(vertexCode.size());
	  shaderDescription[1].shaderStage = ShaderStage::Fragment;
	  shaderDescription[1].code = fragmentCode;
	  shaderDescription[1].sizeInByte = static_cast<uint32_t>(fragmentCode.size());
	  pipelineDesc.shaderStageCount = 2;
	  pipelineDesc.shaderStages = shaderDescription;
	  pipelineDesc.renderPass = context->get_global_renderpass();
	  pipelineDesc.rasterizationState.depthTestFunction = CompareOp::LessOrEqual;
	  pipelineDesc.rasterizationState.enableDepthTest = true;
	  pipelineDesc.rasterizationState.faceCulling = FaceCulling::Back;
	  pipelineDesc.rasterizationState.topology = Topology::Triangle;
	  m_pipeline = Device::create_pipeline(pipelineDesc);


	  m_bindings = Device::create_shader_bindings();
	  m_bindings->set_texture_sampler(m_transmittanceTexture, 0);
	  m_bindings->set_buffer(m_ubo, 1);
  }
}

void Atmosphere::precompute_transmittance(Context* context)
{
	context->begin_compute();
	context->transition_layout_for_compute_read(&m_transmittanceTexture, 1);
	context->update_pipeline(m_transmittanceComputePipeline, &m_transmittanceBindings, 1);
	context->set_pipeline(m_transmittanceComputePipeline);
	//context->set_uniform(ShaderStage::Compute, 0, sizeof(AtmosphereParams), &m_params);
	context->dispatch_compute(m_params.TRANSMITTANCE_TEXTURE_DIM / 32, m_params.TRANSMITTANCE_TEXTURE_DIM / 32, 1);
	context->transition_layout_for_shader_read(&m_transmittanceTexture, 1);
	context->end_compute();
}

void Atmosphere::render(Context* context, Ref<Camera> camera)
{
	context->update_pipeline(m_pipeline, &m_bindings, 1);
	context->set_pipeline(m_pipeline);

	glm::mat4 uniformData[] = { camera->get_inv_projection(), camera->get_inv_view() };
	glm::vec3 cameraPos = camera->get_position();
	cameraPos.y += m_params.EARTH_RADIUS;
	context->set_uniform(ShaderStage::Fragment, 0, sizeof(glm::mat4) * 2, &uniformData[0][0]);
	context->set_uniform(ShaderStage::Fragment, sizeof(glm::mat4) * 2, sizeof(glm::vec3), &cameraPos[0]);
	context->draw(6);
}


void Atmosphere::destroy()
{
	Device::destroy_pipeline(m_transmittanceComputePipeline);
	Device::destroy_texture(m_transmittanceTexture);
	Device::destroy_shader_bindings(m_transmittanceBindings);

	Device::destroy_shader_bindings(m_bindings);
	Device::destroy_pipeline(m_pipeline);
	Device::destroy_buffer(m_ubo);
}

