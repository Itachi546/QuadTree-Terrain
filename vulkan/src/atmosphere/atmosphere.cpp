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
#include "scene/mesh.h"

Atmosphere::Atmosphere(Context* context)
{
	m_params.EARTH_RADIUS = 6.360e6f;
	m_params.ATMOSPHERE_RADIUS = 6.42e6f;

	m_params.Hr = 7.994e3f;
	m_params.Hm = 1.2e3f;

	m_params.BetaR = glm::vec3(5.8e-6f, 13.5e-6f, 33.1e-6f);
	m_params.BetaM = glm::vec3(21e-6f);
	m_params.TRANSMITTANCE_TEXTURE_DIM = 1024;

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
	  //desc.format = Format::R16G16B16A16Float;
	  desc.format = Format::R32G32Float;
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
	  TextureDescription cubemapDesc = TextureDescription::Initialize(CUBEMAP_DIMS, CUBEMAP_DIMS);
	  cubemapDesc.flags = TextureFlag::Sampler | TextureFlag::StorageImage;
	  cubemapDesc.format = Format::R8G8B8A8_Unorm;
	  cubemapDesc.type = TextureType::Cubemap;
	  SamplerDescription sampler = SamplerDescription::Initialize();
	  sampler.wrapU = sampler.wrapV = sampler.wrapW = WrapMode::ClampToEdge;
	  cubemapDesc.sampler = &sampler;
	  m_cubemap = Device::create_texture(cubemapDesc);

	  std::string code = load_file("spirv/atmosphere.comp.spv");
	  PipelineDescription desc = {};
	  ShaderDescription shader = { ShaderStage::Compute, code, static_cast<uint32_t>(code.size()) };
	  desc.shaderStageCount = 1;
	  desc.shaderStages = &shader;
	  m_cubemapPipeline = Device::create_pipeline(desc);

	  m_cubemapBindings = Device::create_shader_bindings();
	  m_cubemapBindings->set_texture_sampler(m_transmittanceTexture, 0);
	  m_cubemapBindings->set_buffer(m_ubo, 1);
	  m_cubemapBindings->set_storage_image(m_cubemap, 2);
  }

  {
	  std::string vertexCode = load_file("spirv/cubemap.vert.spv");
	  ASSERT(vertexCode.size() % 4 == 0);
	  std::string fragmentCode = load_file("spirv/cubemap.frag.spv");
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
	  pipelineDesc.rasterizationState.faceCulling = FaceCulling::Front;
	  pipelineDesc.rasterizationState.topology = Topology::Triangle;
	  m_pipeline = Device::create_pipeline(pipelineDesc);

	  m_bindings = Device::create_shader_bindings();
	  m_bindings->set_texture_sampler(m_cubemap, 0);
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

void Atmosphere::update(Context* context, Ref<Camera> camera, glm::vec3 lightDir, float lightIntensity)
{
	context->begin_compute();
	context->transition_layout_for_compute_read(&m_cubemap, 1);
	context->update_pipeline(m_cubemapPipeline, &m_cubemapBindings, 1);
	context->set_pipeline(m_cubemapPipeline);

	glm::vec4 uniformData[] = {
		glm::vec4(camera->get_position() + vec3(0.0f, m_params.EARTH_RADIUS, 0.0f), float(CUBEMAP_DIMS)),
		glm::vec4(lightDir, lightIntensity),
	};
	context->set_uniform(ShaderStage::Compute, 0, sizeof(glm::vec4) * 2, uniformData);
	context->dispatch_compute(CUBEMAP_DIMS / 32, CUBEMAP_DIMS / 32, 6);
	context->transition_layout_for_shader_read(&m_cubemap, 1);
	context->end_compute();
}

void Atmosphere::render(Context* context, Ref<Mesh> cube, Ref<Camera> camera, glm::vec3 lightDir, float lightIntensity)
{
	context->update_pipeline(m_pipeline, &m_bindings, 1);
	context->set_pipeline(m_pipeline);

	struct UniformData
	{
		glm::mat4 projection;
		glm::mat4 view;
		vec4 cameraPosition;
		vec4 lightDirection;
	} uniformData;
	uniformData.projection = camera->get_projection();
	uniformData.view = camera->get_view();
	uniformData.cameraPosition = glm::vec4(camera->get_position() + vec3(0.0f, m_params.EARTH_RADIUS, 0.0f), float(CUBEMAP_DIMS));
	uniformData.lightDirection = glm::vec4(lightDir, lightIntensity);

	context->set_uniform(ShaderStage::Vertex, 0, sizeof(uniformData), &uniformData);

	VertexBufferView* vb = cube->get_vb();
	context->set_buffer(vb->buffer, vb->offset);
	IndexBufferView* ib = cube->get_ib();
	context->set_buffer(ib->buffer, ib->offset);
	context->draw_indexed(cube->get_indices_count());
}


void Atmosphere::destroy()
{
	Device::destroy_pipeline(m_transmittanceComputePipeline);
	Device::destroy_texture(m_transmittanceTexture);
	Device::destroy_texture(m_cubemap);
	Device::destroy_shader_bindings(m_transmittanceBindings);

	Device::destroy_shader_bindings(m_bindings);
	Device::destroy_pipeline(m_pipeline);

	Device::destroy_shader_bindings(m_cubemapBindings);
	Device::destroy_pipeline(m_cubemapPipeline);
	Device::destroy_buffer(m_ubo);

}
