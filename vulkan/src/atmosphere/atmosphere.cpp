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
#include "utils/diffuse_irradiance_generator.h"

Pipeline* create_compute_pipeline(const std::string& filename)
{
	std::string code = load_file(filename.c_str());
	PipelineDescription desc = {};
	ShaderDescription shader = { ShaderStage::Compute, code, static_cast<uint32_t>(code.size()) };
	desc.shaderStageCount = 1;
	desc.shaderStages = &shader;
	return Device::create_pipeline(desc);
}

Texture* create_texture(uint32_t size, Format format, TextureType type)
{
	TextureDescription desc = TextureDescription::Initialize(size, size);
	desc.flags = TextureFlag::Sampler | TextureFlag::StorageImage;
	//desc.format = Format::R16G16B16A16Float;
	desc.format = format;
	desc.type = type;
	SamplerDescription sampler = SamplerDescription::Initialize();
	sampler.wrapU = sampler.wrapV = sampler.wrapW = WrapMode::ClampToEdge;
	desc.sampler = &sampler;
	return Device::create_texture(desc);
}

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

	m_transmittanceInfo.pipeline = create_compute_pipeline("spirv/precompute_transmittance.comp.spv");
	m_transmittanceInfo.texture = create_texture(m_params.TRANSMITTANCE_TEXTURE_DIM, Format::R32G32Float, TextureType::Color2D);
	m_transmittanceInfo.bindings = Device::create_shader_bindings();
	m_transmittanceInfo.bindings->set_storage_image(m_transmittanceInfo.texture, 0);
	m_transmittanceInfo.bindings->set_buffer(m_ubo, 1);
	precompute_transmittance(context);


	// Generate Cubemap texture
	m_atmosphereCubemapInfo.texture = create_texture(CUBEMAP_DIMS, Format::R16G16B16A16Float, TextureType::Cubemap);
	m_atmosphereCubemapInfo.pipeline = create_compute_pipeline("spirv/atmosphere.comp.spv");

	m_atmosphereCubemapInfo.bindings = Device::create_shader_bindings();
	m_atmosphereCubemapInfo.bindings->set_texture_sampler(m_transmittanceInfo.texture, 0);
	m_atmosphereCubemapInfo.bindings->set_buffer(m_ubo, 1);
	m_atmosphereCubemapInfo.bindings->set_storage_image(m_atmosphereCubemapInfo.texture, 2);
	
	m_diffuseIrradianceInfo.texture = create_texture(DIFFUSE_IRRADIANCE_DIMS, Format::R16G16B16A16Float, TextureType::Cubemap);
	m_diffuseIrradianceInfo.bindings = Device::create_shader_bindings();
	m_diffuseIrradianceInfo.bindings->set_storage_image(m_atmosphereCubemapInfo.texture, 0);
	m_diffuseIrradianceInfo.bindings->set_storage_image(m_diffuseIrradianceInfo.texture, 1);
	m_DIGenerator = CreateRef<DiffuseIrradianceGenerator>();


	// Skybox renderer pipeline
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
	m_skyboxPipeline = Device::create_pipeline(pipelineDesc);
	m_skyboxBindings = Device::create_shader_bindings();
	m_skyboxBindings->set_texture_sampler(m_atmosphereCubemapInfo.texture, 0);
}

void Atmosphere::precompute_transmittance(Context* context)
{
	Texture* texture = m_transmittanceInfo.texture;
	ShaderBindings* bindings = m_transmittanceInfo.bindings;
	Pipeline* pipeline = m_transmittanceInfo.pipeline;

	context->begin_compute();
	context->transition_layout_for_compute_read(&texture, 1);
	context->update_pipeline(pipeline, &bindings, 1);
	context->set_pipeline(pipeline);
	//context->set_uniform(ShaderStage::Compute, 0, sizeof(AtmosphereParams), &m_params);
	context->dispatch_compute(m_params.TRANSMITTANCE_TEXTURE_DIM / 32, m_params.TRANSMITTANCE_TEXTURE_DIM / 32, 1);
	context->transition_layout_for_shader_read(&texture, 1);
	context->end_compute();
}

void Atmosphere::update(Context* context, Ref<Camera> camera, glm::vec3 lightDir, float lightIntensity)
{
	context->begin_compute();

	Texture* textures[] = {
		m_atmosphereCubemapInfo.texture,
		m_diffuseIrradianceInfo.texture
	};
	context->transition_layout_for_compute_read(textures, ARRAYSIZE(textures));

	// Render atmosphere to cubemap
	{
		context->update_pipeline(m_atmosphereCubemapInfo.pipeline, &m_atmosphereCubemapInfo.bindings, 1);
		context->set_pipeline(m_atmosphereCubemapInfo.pipeline);

		glm::vec4 uniformData[] = {
			glm::vec4(camera->get_position() + vec3(0.0f, m_params.EARTH_RADIUS, 0.0f), float(CUBEMAP_DIMS)),
			glm::vec4(lightDir, lightIntensity),
		};
		context->set_uniform(ShaderStage::Compute, 0, sizeof(glm::vec4) * 2, uniformData);
		context->dispatch_compute(CUBEMAP_DIMS / 32, CUBEMAP_DIMS / 32, 6);
	}

	// Filter the atmosphere skybox
	{
		m_DIGenerator->generate(context, &m_diffuseIrradianceInfo.bindings, 1, DIFFUSE_IRRADIANCE_DIMS);
	}
	context->transition_layout_for_shader_read(textures, ARRAYSIZE(textures));
	context->end_compute();
}

void Atmosphere::render(Context* context, Ref<Mesh> cube, Ref<Camera> camera, glm::vec3 lightDir, float lightIntensity)
{
	context->update_pipeline(m_skyboxPipeline, &m_skyboxBindings, 1);
	context->set_pipeline(m_skyboxPipeline);

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
	Device::destroy_pipeline(m_transmittanceInfo.pipeline);
	Device::destroy_texture(m_transmittanceInfo.texture);
	Device::destroy_shader_bindings(m_transmittanceInfo.bindings);

	Device::destroy_pipeline(m_atmosphereCubemapInfo.pipeline);
	Device::destroy_texture(m_atmosphereCubemapInfo.texture);
	Device::destroy_shader_bindings(m_atmosphereCubemapInfo.bindings);

	Device::destroy_pipeline(m_skyboxPipeline);
	Device::destroy_shader_bindings(m_skyboxBindings);

	Device::destroy_texture(m_diffuseIrradianceInfo.texture);
	Device::destroy_shader_bindings(m_diffuseIrradianceInfo.bindings);

	m_DIGenerator->destroy();
	Device::destroy_buffer(m_ubo);

}

