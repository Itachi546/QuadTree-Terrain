#include "grass.h"
#include "renderer/pipeline.h"
#include "renderer/context.h"
#include "renderer/device.h"
#include "common/common.h"
#include "renderer/shaderbinding.h"
#include "renderer/texture.h"
#include "scene/entity.h"
#include "common/image_loader.h"
#include "terrain_chunk.h"

Texture* create_texture(Context* context, const char* filename)
{
	int width, height, nChannel;
	unsigned char* buffer = ImageLoader::load_image(filename, &width, &height, &nChannel);

	TextureDescription desc = TextureDescription::Initialize(width, height);

	if (nChannel == 4)
		desc.format = Format::R8G8B8A8_Unorm;
	else if (nChannel == 3)
		desc.format = Format::R8G8B8_Unorm;
	else
		ASSERT(0);
	desc.flags = TextureFlag::Sampler | TextureFlag::TransferDst;
	SamplerDescription sampler = SamplerDescription::Initialize();
	sampler.wrapU = sampler.wrapV = sampler.wrapW = WrapMode::Repeat;
	desc.sampler = &sampler;
	Texture* texture = Device::create_texture(desc);
	context->copy(texture, buffer, width * height * nChannel);
	return texture;
}

Grass::Grass(Context* context)
{
	std::string vertexCode = load_file("spirv/grass.vert.spv");
	ASSERT(vertexCode.size() % 4 == 0);
	std::string fragmentCode = load_file("spirv/grass.frag.spv");
	ASSERT(fragmentCode.size() % 4 == 0);
	std::string geometryCode = load_file("spirv/grass.geom.spv");
	ASSERT(geometryCode.size() % 4 == 0);

	PipelineDescription pipelineDesc = {};
	ShaderDescription shaderDescription[3] = {};
	shaderDescription[0].shaderStage = ShaderStage::Vertex;
	shaderDescription[0].code = vertexCode;
	shaderDescription[0].sizeInByte = static_cast<uint32_t>(vertexCode.size());

	shaderDescription[1].shaderStage = ShaderStage::Geometry;
	shaderDescription[1].code = geometryCode;
	shaderDescription[1].sizeInByte = static_cast<uint32_t>(geometryCode.size());

	shaderDescription[2].shaderStage = ShaderStage::Fragment;
	shaderDescription[2].code = fragmentCode;
	shaderDescription[2].sizeInByte = static_cast<uint32_t>(fragmentCode.size());


	pipelineDesc.shaderStageCount = 3;
	pipelineDesc.shaderStages = shaderDescription;
	pipelineDesc.renderPass = context->get_global_renderpass();
	pipelineDesc.rasterizationState.depthTestFunction = CompareOp::LessOrEqual;
	pipelineDesc.rasterizationState.enableDepthTest = true;
	pipelineDesc.rasterizationState.faceCulling = FaceCulling::None;
	pipelineDesc.rasterizationState.topology = Topology::Triangle;
	m_pipeline = Device::create_pipeline(pipelineDesc);

	m_distortionTexture = create_texture(context, "assets/distortion.png");
	m_noiseTexture = create_texture(context, "assets/distortion.png");

	Texture* textures[] = {
		m_distortionTexture,
		m_noiseTexture
	};

	//@TODO Nasty little hack to setup temporary command buffer
	context->begin_compute();
	context->transition_layout_for_shader_read(textures, ARRAYSIZE(textures));
	context->end_compute();
	//context->transition_layout_for_shader_read(&m_distortionTexture, 1);

	m_bindings = Device::create_shader_bindings();
	m_bindings->set_texture_sampler(m_distortionTexture, 4);
	m_bindings->set_texture_sampler(m_noiseTexture, 5);
}

void Grass::render(Context* context, Entity* entity, ShaderBindings** bindings, uint32_t count, float elapsedTime)
{
	std::vector<ShaderBindings*> totalBindings;
	for (uint32_t i = 0; i < count; ++i)
		totalBindings.push_back(*(bindings + i));
	totalBindings.push_back(m_bindings);

	context->update_pipeline(m_pipeline, totalBindings.data(), static_cast<uint32_t>(totalBindings.size()));
	context->set_pipeline(m_pipeline);

	VertexBufferView* vb = entity->mesh->get_vb();
	IndexBufferView* ib = entity->mesh->get_ib();

	glm::mat4 model = entity->transform->get_mat4();
	context->set_uniform(ShaderStage::Vertex, 0, sizeof(glm::mat4), &model[0][0]);
	context->set_uniform(ShaderStage::Geometry, sizeof(glm::mat4), sizeof(float), &elapsedTime);

	context->set_buffer(vb->buffer, vb->offset);
	context->set_buffer(ib->buffer, ib->offset);
	context->draw_indexed(entity->mesh->get_indices_count());
}

void Grass::render(Context* context, std::vector<TerrainChunk*> chunks, IndexBuffer* ib, uint32_t indicesCount, ShaderBindings** bindings, uint32_t count, float elapsedTime)
{
	std::vector<ShaderBindings*> totalBindings;
	for (uint32_t i = 0; i < count; ++i)
		totalBindings.push_back(*(bindings + i));
	totalBindings.push_back(m_bindings);

	context->update_pipeline(m_pipeline, totalBindings.data(), static_cast<uint32_t>(totalBindings.size()));
	context->set_pipeline(m_pipeline);

	glm::mat4 model = glm::mat4(1.0f);
	context->set_uniform(ShaderStage::Vertex, 0, sizeof(glm::mat4), &model[0][0]);
	context->set_uniform(ShaderStage::Geometry, sizeof(glm::mat4), sizeof(float), &elapsedTime);
	context->set_buffer(ib, 0);
	for (auto& chunk : chunks)
	{
		Ref<VertexBufferView> vb = chunk->vb;
		context->set_buffer(vb->buffer, vb->offset);
		context->draw_indexed(indicesCount);
	}
}

void Grass::destroy()
{
	Device::destroy_texture(m_noiseTexture);
	Device::destroy_pipeline(m_pipeline);
	Device::destroy_texture(m_distortionTexture);
	Device::destroy_shader_bindings(m_bindings);
}
