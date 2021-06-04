#include "outlinefx.h"
#include "common/common.h"
#include "renderer/renderpass.h"
#include "renderer/pipeline.h"
#include "renderer/device.h"
#include "renderer/context.h"
#include "scene/gpu_memory.h"
#include "scene/entity.h"
#include "renderer/shaderbinding.h"
#include "renderer/texture.h"
#include "renderer/framebuffer.h"

void OutlineFX::init(uint32_t width, uint32_t height)
{
	m_width = width;
	m_height = height;

	// Create Renderpass
	RenderPassDescription renderPassDesc = {};
	Attachment attachments[] = {
		Attachment{0, Format::R8G8B8A8_Unorm, TextureType::Color2D},
	};
	renderPassDesc.attachmentCount = 1;
	renderPassDesc.attachments = attachments;
	renderPassDesc.width = width;
	renderPassDesc.height = height;
	m_renderpass = Device::create_renderpass(renderPassDesc);

	// Create Framebuffer
	TextureDescription textureDesc = {};
	textureDesc.format = Format::R8G8B8A8_Unorm;
	textureDesc.height = height;
	textureDesc.width = width;
	textureDesc.type = TextureType::Color2D;
	textureDesc.flags = TextureFlag::Sampler;
	SamplerDescription sampler = SamplerDescription::Initialize();
	textureDesc.sampler = &sampler;

	FramebufferDescription framebufferDesc = {};
	framebufferDesc.attachmentCount = 1;
	framebufferDesc.width = width;
	framebufferDesc.height = height;
	framebufferDesc.attachments = &textureDesc;

	m_prepassAttachment = Device::create_framebuffer(framebufferDesc, m_renderpass);
	m_outlineAttachment = Device::create_framebuffer(framebufferDesc, m_renderpass);

	// Setup pipeline
	std::string vertexCode = load_file("spirv/outline.vert.spv");
	ASSERT(vertexCode.size() % 4 == 0);
	std::string fragmentCode = load_file("spirv/outline.frag.spv");
	ASSERT(fragmentCode.size() % 4 == 0);

	ShaderDescription stages[2] = {
		ShaderDescription{ShaderStage::Vertex, vertexCode, static_cast<uint32_t>(vertexCode.size())},
		ShaderDescription{ShaderStage::Fragment, fragmentCode, static_cast<uint32_t>(fragmentCode.size())}};
	
	PipelineDescription desc = {};
	desc.shaderStageCount = 2;
	desc.shaderStages = stages;
	desc.rasterizationState.enableDepthTest = false;
	desc.renderPass = m_renderpass;
	m_prepassPipeline = Device::create_pipeline(desc);

	vertexCode = load_file("spirv/image2d.vert.spv");
	ASSERT(vertexCode.size() % 4 == 0);
	fragmentCode = load_file("spirv/edge_detection.frag.spv");
	ASSERT(fragmentCode.size() % 4 == 0);

	stages[0] = ShaderDescription{ ShaderStage::Vertex, vertexCode, static_cast<uint32_t>(vertexCode.size()) },
	stages[1] = ShaderDescription{ ShaderStage::Fragment, fragmentCode, static_cast<uint32_t>(fragmentCode.size()) };

	desc.shaderStageCount = 2;
	desc.shaderStages = stages;
	desc.renderPass = m_renderpass;
	m_outlinePipeline = Device::create_pipeline(desc);

	m_bindings = Device::create_shader_bindings();
	m_bindings->set_texture_sampler(m_prepassAttachment->get_color_attachment(0), 0);

	m_outlineOutputBindings = Device::create_shader_bindings();
	m_outlineOutputBindings->set_texture_sampler(m_outlineAttachment->get_color_attachment(0), 0);
}

void OutlineFX::resize(uint32_t width, uint32_t height)
{
	//m_renderpass->resize(width, height);
	//m_edgeDetectionPass->resize(width, height);

	m_width = width;
	m_height = height;
}

void OutlineFX::render(Context* context, std::vector<Entity*> entities, ShaderBindings* globalBindings)
{
	context->set_clear_color(0.0f, 0.0f, 0.0f, 1.0f);
	context->begin_renderpass(m_renderpass, m_prepassAttachment);
	context->set_pipeline(m_prepassPipeline);
	ShaderBindings* bindingArr[] = { globalBindings };
	context->set_shader_bindings(bindingArr, 1);
	for (Entity* entity : entities)
	{
		glm::mat4 model = entity->transform->get_mat4();
		context->set_uniform(ShaderStage::Vertex, 0, sizeof(glm::mat4), &model[0][0]);
		Ref<Mesh> mesh = entity->mesh;

		VertexBufferView* vb = mesh->get_vb();
		context->set_buffer(vb->buffer, vb->offset);

		IndexBufferView* ib = mesh->get_ib();
		context->set_buffer(ib->buffer, ib->offset);
		context->draw_indexed(mesh->get_indices_count());
	}
	context->end_renderpass();

	Texture* texture = m_prepassAttachment->get_color_attachment(0);
	context->transition_layout_for_shader_read(&texture, 1);

	context->set_clear_color(0.0f, 0.0f, 0.0f, 1.0f);
	context->begin_renderpass(m_renderpass, m_outlineAttachment);
	context->set_pipeline(m_outlinePipeline);
	glm::vec2 inv_resolution = glm::vec2(1.0f / float(m_width), 1.0f / float(m_height));
	context->set_uniform(ShaderStage::Fragment, 0, sizeof(glm::vec2), &inv_resolution);
	context->set_shader_bindings(&m_bindings, 1);
	context->draw(6);
	context->end_renderpass();

	texture = m_outlineAttachment->get_color_attachment(0);
	context->transition_layout_for_shader_read(&texture, 1);
}

void OutlineFX::destroy()
{
	Device::destroy_shader_bindings(m_bindings);
	Device::destroy_pipeline(m_prepassPipeline);
	Device::destroy_pipeline(m_outlinePipeline);
	Device::destroy_framebuffer(m_prepassAttachment);
	Device::destroy_framebuffer(m_outlineAttachment);
	Device::destroy_renderpass(m_renderpass);
}
