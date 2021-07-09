#include "skybox.h"
#include "renderer/texture.h"
#include "common/image_loader.h"
#include "renderer/device.h"
#include "renderer/context.h"
#include "common/common.h"
#include "renderer/pipeline.h"
#include "renderer/shaderbinding.h"
#include "diffuse_irradiance_generator.h"
#include "core/math.h"
#include "scene/camera.h"
#include "scene/mesh.h"
#include "atmosphere/atmosphere.h"
#include "light/directional_light.h"

void Skybox::initialize(Context* context)
{
	glm::mat4 view[] = {
		glm::lookAt(glm::vec3(0.0), glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)),
		glm::lookAt(glm::vec3(0.0), glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0)),
		glm::lookAt(glm::vec3(0.0), glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0,  0.0, -1.0)),
		glm::lookAt(glm::vec3(0.0), glm::vec3(0.0,  1.0, 0.0), glm::vec3(0.0, 0.0,  1.0)),
		glm::lookAt(glm::vec3(0.0), glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)),
		glm::lookAt(glm::vec3(0.0), glm::vec3(0.0, 0.0,-1.0), glm::vec3(0.0, -1.0, 0.0)),
		glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f)
	};

	m_ubo = Device::create_uniformbuffer(BufferUsageHint::StaticDraw, sizeof(glm::mat4) * 7);
	context->copy(m_ubo, &view[0][0], 0, sizeof(glm::mat4) * 7);
	m_cubemap = create_cubemap_texture(context, CUBEMAP_SIZE);
	m_irradiance = create_cubemap_texture(context, IRRADIANCE_CUBMAP_SIZE);

	m_irradianceBindings = Device::create_shader_bindings();
	m_irradianceBindings->set_storage_image(m_cubemap, 0);
	m_irradianceBindings->set_storage_image(m_irradiance, 1);
	m_irradianceBindings->set_buffer(m_ubo, 2);
	m_irradianceGenerator = CreateRef<DiffuseIrradianceGenerator>();
	setup_render_pipeline(context);
}

Skybox::Skybox(Context* context, const char* hdriFile)
{
	CUBEMAP_SIZE = 1024;
	initialize(context);
	Texture* hdri = load_hdri(context, hdriFile);
	convert_hdri_to_cubemap(context, hdri);
	Device::destroy_texture(hdri);
}

Skybox::Skybox(Context* context)
{
	CUBEMAP_SIZE = 256;
	initialize(context);
	m_atmosphere = CreateRef<Atmosphere>(context, m_cubemap, m_ubo);

}

void Skybox::update(Context* context, Ref<Camera> camera, Ref<DirectionalLight> sun)
{
	context->begin_compute();
	Texture* textures[] = { m_cubemap, m_irradiance };
	context->transition_layout_for_compute_read(textures, ARRAYSIZE(textures));

	if (m_atmosphere)
		m_atmosphere->update(context, camera, sun->get_direction(), sun->get_intensity());

	m_irradianceGenerator->generate(context, &m_irradianceBindings, 1, IRRADIANCE_CUBMAP_SIZE);
	context->transition_layout_for_shader_read(textures, ARRAYSIZE(textures));
	context->end_compute();
}

void Skybox::render(Context* context, Ref<Camera> camera, Ref<Mesh> cubeMesh)
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

	context->set_uniform(ShaderStage::Vertex, 0, sizeof(uniformData), &uniformData);
	VertexBufferView* vb = cubeMesh->get_vb();
	context->set_buffer(vb->buffer, vb->offset);
	IndexBufferView* ib = cubeMesh->get_ib();
	context->set_buffer(ib->buffer, ib->offset);
	context->draw_indexed(cubeMesh->get_indices_count());
}

Texture* Skybox::create_cubemap_texture(Context* context, uint32_t size)
{
	TextureDescription desc = TextureDescription::Initialize(size, size);
	desc.flags = TextureFlag::Sampler | TextureFlag::StorageImage;
	//desc.format = Format::R16G16B16A16Float;
	desc.format = Format::R32G32B32A32Float;
	desc.type = TextureType::Cubemap;
	SamplerDescription sampler = SamplerDescription::Initialize();
	sampler.wrapU = sampler.wrapV = sampler.wrapW = WrapMode::ClampToEdge;
	desc.sampler = &sampler;
	return Device::create_texture(desc);
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
	bindings->set_buffer(m_ubo, 2);

	context->begin_compute();
	context->transition_layout_for_shader_read(&hdri, 1);
	context->transition_layout_for_compute_read(&m_cubemap, 1);
	context->update_pipeline(pipeline, &bindings, 1);
	context->set_pipeline(pipeline);

	float size[] = {float(CUBEMAP_SIZE), float(CUBEMAP_SIZE)};
	context->set_uniform(ShaderStage::Compute, 0, sizeof(float) * 2, size);
	context->dispatch_compute(CUBEMAP_SIZE / 32, CUBEMAP_SIZE / 32, 6);
	context->end_compute();

	Device::destroy_pipeline(pipeline);
	Device::destroy_shader_bindings(bindings);
}

void Skybox::setup_render_pipeline(Context* context)
{
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
	m_pipeline = Device::create_pipeline(pipelineDesc);
	m_bindings = Device::create_shader_bindings();
	m_bindings->set_texture_sampler(m_cubemap, 0);
}

void Skybox::destroy()
{

	Device::destroy_texture(m_cubemap);
	Device::destroy_texture(m_irradiance);
	Device::destroy_buffer(m_ubo);
	m_irradianceGenerator->destroy();

	Device::destroy_pipeline(m_pipeline);
	Device::destroy_shader_bindings(m_bindings);

	if(m_atmosphere)
		m_atmosphere->destroy();
}

