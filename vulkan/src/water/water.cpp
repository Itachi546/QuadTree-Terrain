#include "water.h"
#include "spectrum_texture.h"
#include "twiddle_factors.h"
#include "inversion.h"
#include "butterfly.h"
#include "normal_map_generator.h"
#include "water_renderer.h"

#include "renderer/context.h"
#include "renderer/device.h"
#include "renderer/shaderbinding.h"
#include "renderer/renderpass.h"
#include "renderer/framebuffer.h"
#include "renderer/pipeline.h"
#include "renderer/buffer.h"
#include "common/common.h"

#include "core/frustum.h"
#include "scene/scene.h"
#include "scene/camera.h"
#include "scene/mesh.h"
#include "terrain/terrain.h"
#include <imgui/imgui.h>
#include "utils/skybox.h"

Pipeline* Water::create_atmosphere_pipeline(Context* context, const std::string& vertexCode, const std::string& fragmentCode)
{
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
	pipelineDesc.renderPass = m_renderPass;
	pipelineDesc.rasterizationState.depthTestFunction = CompareOp::LessOrEqual;
	pipelineDesc.rasterizationState.enableDepthTest = true;
	pipelineDesc.rasterizationState.faceCulling = FaceCulling::Front;
	pipelineDesc.rasterizationState.topology = Topology::Triangle;

	return Device::create_pipeline(pipelineDesc);
}

void Water::create_renderpass(Context* context)
{
	RenderPassDescription desc = {};

	Attachment attachments[] = {
		Attachment{0, Format::R8G8B8A8_Unorm, TextureType::Color2D},
		Attachment{1, Format::D32Float, TextureType::DepthStencil}
	};
	desc.attachments = attachments;
	desc.attachmentCount = ARRAYSIZE(attachments);
	desc.width = OFFSCREEN_WIDTH;
	desc.height = OFFSCREEN_HEIGHT;
	m_renderPass = Device::create_renderpass(desc);
}


Framebuffer* Water::create_framebuffer(Context* context)
{
	FramebufferDescription desc = {};

	uint32_t width = OFFSCREEN_WIDTH;
	uint32_t height = OFFSCREEN_HEIGHT;

	TextureDescription colorAttachment = TextureDescription::Initialize(width, height);
	colorAttachment.flags = TextureFlag::Sampler;
	colorAttachment.format = Format::R8G8B8A8_Unorm;
	colorAttachment.type = TextureType::Color2D;
	SamplerDescription sampler = SamplerDescription::Initialize();
	sampler.wrapU = sampler.wrapV = sampler.wrapW = WrapMode::Repeat;
	colorAttachment.sampler = &sampler;
	
	TextureDescription depthAttachment = TextureDescription::Initialize(width, height);
	depthAttachment.format = Format::D32Float;
	depthAttachment.type = TextureType::DepthStencil;
	depthAttachment.flags = TextureFlag::Sampler;
	depthAttachment.sampler = &sampler;

	TextureDescription attachments[] = { colorAttachment, depthAttachment };
	desc.attachments = attachments;
	desc.attachmentCount = ARRAYSIZE(attachments);
	desc.width = width;
	desc.height = height;

	return Device::create_framebuffer(desc, m_renderPass);
}

Pipeline* Water::create_pipeline(Context* context, const std::string& vertexCode, const std::string& fragmentCode)
{
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
	pipelineDesc.renderPass = m_renderPass;
	pipelineDesc.rasterizationState.depthTestFunction = CompareOp::LessOrEqual;
	pipelineDesc.rasterizationState.enableDepthTest = true;
	pipelineDesc.rasterizationState.faceCulling = FaceCulling::Back;
	pipelineDesc.rasterizationState.polygonMode = PolygonMode::Fill;
	pipelineDesc.rasterizationState.topology = Topology::Triangle;
	return Device::create_pipeline(pipelineDesc);
}

Water::Water(Context* context)
{
	m_reflection.ubo = Device::create_uniformbuffer(BufferUsageHint::DynamicCopy, sizeof(OffscreenUniformData));
	m_reflection.binding = Device::create_shader_bindings();
	m_reflection.binding->set_buffer(m_reflection.ubo, 0);

	m_refraction.ubo = Device::create_uniformbuffer(BufferUsageHint::DynamicCopy, sizeof(OffscreenUniformData));
	m_refraction.binding = Device::create_shader_bindings();
	m_refraction.binding->set_buffer(m_refraction.ubo, 0);

	m_properties = CreateRef<WaterProperties>();
	// @TODO support other dimension by updating the 
	// generation of bit reversed indices

	m_properties->dimension = 256;
	m_properties->horizontalDimension = 1024;
	m_properties->windDirection = glm::vec2(1.0f, 0.0f);
	m_properties->windSpeed = 200.0f;
	m_properties->philipAmplitude = 2.0f;

	m_spectrumTexture = CreateRef<SpectrumTexture>(context, m_properties);
	m_spectrumTexture->create_spectrum_texture(context, m_properties);

	m_butterflyTexture = CreateRef<TwiddleFactors>(context, m_properties->dimension);
	m_butterflyTexture->create_twiddle_texture(context);

	{
		m_fft = CreateRef<ButterflyOperation>(context, m_properties->dimension);
		m_fftBindings = Device::create_shader_bindings();
		m_fftBindings->set_storage_image(m_butterflyTexture->get_twiddle_texture(), 0);
		m_fftBindings->set_storage_image(m_spectrumTexture->get_pingpoing_texture0(), 1);
		m_fftBindings->set_storage_image(m_spectrumTexture->get_pingpoing_texture1(), 2);
	}

	{
		// Create Reflection and Refraction RenderPass
		create_renderpass(context);

		std::string vertexCode = load_file("spirv/water_offscreen.vert.spv");
		ASSERT(vertexCode.size() % 4 == 0);
		std::string fragmentCode = load_file("spirv/water_offscreen.frag.spv");
		ASSERT(fragmentCode.size() % 4 == 0);
		m_reflection.meshPipeline = create_pipeline(context, vertexCode, fragmentCode);
		m_refraction.meshPipeline = create_pipeline(context, vertexCode, fragmentCode);

		vertexCode = load_file("spirv/water_offscreen_terrain.vert.spv");
		ASSERT(vertexCode.size() % 4 == 0);
		fragmentCode = load_file("spirv/water_offscreen_terrain.frag.spv");
		ASSERT(fragmentCode.size() % 4 == 0);
		m_reflection.terrainPipeline = create_pipeline(context, vertexCode, fragmentCode);
		m_refraction.terrainPipeline = create_pipeline(context, vertexCode, fragmentCode);

		vertexCode = load_file("spirv/cubemap.vert.spv");
		ASSERT(vertexCode.size() % 4 == 0);
		fragmentCode = load_file("spirv/cubemap.frag.spv");
		ASSERT(fragmentCode.size() % 4 == 0);
		m_offscreenCubemapPipeline = create_atmosphere_pipeline(context, vertexCode, fragmentCode);

		m_reflection.fb = create_framebuffer(context);
		m_refraction.fb = create_framebuffer(context);
	}

	m_inversion = CreateRef<Inversion>(context, m_properties->dimension, m_spectrumTexture->get_pingpoing_texture0(), m_spectrumTexture->get_pingpoing_texture1());
	m_normaMapGenerator = CreateRef<NormalMapGenerator>(context, m_inversion->get_height_texture());

	m_rendererBindings = Device::create_shader_bindings();
	m_rendererBindings->set_texture_sampler(m_inversion->get_height_texture(), 2);
	m_rendererBindings->set_texture_sampler(m_normaMapGenerator->get_normal_texture(), 3);
	m_rendererBindings->set_texture_sampler(m_reflection.fb->get_color_attachment(0), 4);
	m_rendererBindings->set_texture_sampler(m_refraction.fb->get_color_attachment(0), 5);
	m_rendererBindings->set_texture_sampler(m_refraction.fb->get_depth_attachment(), 6);
	m_renderer = CreateRef<WaterRenderer>(context);

	debugBindings = Device::create_shader_bindings();
	//debugBindings->set_texture_sampler(m_normaMapGenerator->get_normal_texture(), 0);
	debugBindings->set_texture_sampler(m_refraction.fb->get_color_attachment(0), 0);
	{
		m_waterParams.maxDepth = 100.0f;
		m_waterParams.maxFoamDepth = 0.05f;
		m_waterParams.foamColor = glm::vec3(1.0f);
		m_waterParams.absorptionColor = m_waterParams.waterColor = glm::vec3(0.1f, 0.3f, 0.7f);
		m_waterParams.shoreBlendDistance = 10.0f;
		m_waterUniformParams = Device::create_uniformbuffer(BufferUsageHint::DynamicCopy, sizeof(m_waterParams));
		m_rendererBindings->set_buffer(m_waterUniformParams, 7);
	}
}

void Water::update(Context* context, float dt)
{
	m_timeElapsed += dt;
	context->begin_compute();
	m_spectrumTexture->create_hdt_texture(context, m_timeElapsed, m_properties->horizontalDimension);
	m_fft->update(context, m_fftBindings, m_properties->dimension);
	m_inversion->update(context, m_properties->dimension, m_fft->get_texture_index());

	Texture* textures[] = {
			m_spectrumTexture->get_pingpoing_texture0(),
			m_spectrumTexture->get_pingpoing_texture1(),
			m_inversion->get_height_texture(),
	};
	context->transition_layout_for_shader_read(textures, ARRAYSIZE(textures));
	m_normaMapGenerator->generate(context);

	Texture* normalMap = m_normaMapGenerator->get_normal_texture();
	context->transition_layout_for_shader_read(&normalMap, 1);
	context->end_compute();

}

void Water::render(Context* context, Ref<Camera> camera, ShaderBindings** uniformBindings, uint32_t count)
{
	if (ImGui::CollapsingHeader("Water"))
	{
		ImGui::SliderFloat("Absorption Max Depth", &m_waterParams.maxDepth, 0.0f, 200.0f);
		ImGui::SliderFloat("Shored Blend Distance", &m_waterParams.shoreBlendDistance, 0.0f, 100.0f);
		ImGui::SliderFloat("Water Height", &m_translate.y, -300, 300);
		ImGui::ColorPicker3("Water Color", &m_waterParams.waterColor[0]);
		ImGui::ColorPicker3("Absorption Color", &m_waterParams.absorptionColor[0]);

	}

	m_waterParams.zN = camera->get_near_plane();
	m_waterParams.zF = camera->get_far_plane();
	context->copy(m_waterUniformParams, &m_waterParams, 0, sizeof(m_waterParams));

	std::vector<ShaderBindings*> bindings;
	for (uint32_t i = 0; i < count; ++i)
		bindings.push_back(uniformBindings[i]);
	bindings.push_back(m_rendererBindings);

	m_renderer->render(context, bindings.data(), camera->get_position(), m_translate, static_cast<uint32_t>(bindings.size()));
}

void Water::prepass(Context* context, Scene* scene, ShaderBindings** bindings, uint32_t count)
{
	std::vector<ShaderBindings*> updatedBindings;
	for (uint32_t i = 0; i < count; ++i)
		updatedBindings.push_back(*bindings + i);
	updatedBindings.push_back(m_reflection.binding);
	uint32_t bindingCount = static_cast<uint32_t>(updatedBindings.size());

	Ref<Camera> camera = scene->get_camera();
	OffscreenUniformData uniformData;
	uniformData.P = camera->get_projection();

	// Reflection Data
	{
		glm::vec3 position = camera->get_position();
		float distance = 2.0f * (position.y - m_translate.y);
		position.y -= distance;

		glm::vec3 rotation = camera->get_rotation();
		glm::mat3 rotate = glm::yawPitchRoll(rotation.y, -rotation.x, rotation.z);
		glm::vec3 up = glm::normalize(rotate * vec3(0.0f, 1.0f, 0.0f));
		glm::vec3 forward = glm::normalize(rotate * vec3(0.0f, 0.0f, 1.0f));

		Ref<Camera> newCamera = camera->clone();
		newCamera->set_rotation(glm::vec3(-rotation.x, rotation.y, rotation.z));
		newCamera->set_position(position);
		newCamera->update(1.0f);

		uniformData.V = newCamera->get_view();
		uniformData.clipPlane = glm::vec4(0.0f, 1.0f, 0.0f, -m_translate.y - 1.0);
		uniformData.cameraPosition = newCamera->get_position();
		context->copy(m_reflection.ubo, &uniformData, 0, sizeof(uniformData));

		context->update_pipeline(m_reflection.meshPipeline, updatedBindings.data(), bindingCount);
		context->update_pipeline(m_reflection.terrainPipeline, updatedBindings.data(), bindingCount);
		generate_offscreen_texture(context, scene, &m_reflection, newCamera, true);
	}

	// Refraction Data
	{
		updatedBindings[count] = m_refraction.binding;
		uniformData.V = camera->get_view();
		uniformData.clipPlane = glm::vec4(0.0f, -1.0f, 0.0f, m_translate.y + 1.0);
		uniformData.cameraPosition = camera->get_position();
		context->copy(m_refraction.ubo, &uniformData, 0, sizeof(uniformData));

		context->update_pipeline(m_refraction.meshPipeline, updatedBindings.data(), bindingCount);
		context->update_pipeline(m_refraction.terrainPipeline, updatedBindings.data(), bindingCount);
		generate_offscreen_texture(context, scene, &m_refraction, camera, false);
		//generate_refraction_texture(context, scene);
	}

	Texture* textures[] = { m_reflection.fb->get_color_attachment(0), m_refraction.fb->get_color_attachment(0), m_refraction.fb->get_depth_attachment()};
	context->transition_layout_for_shader_read(textures, ARRAYSIZE(textures));
}

void Water::generate_offscreen_texture(Context* context, Scene* scene, OffscreenPipelineInfo* info,	Ref<Camera> camera,	bool reflectionPass)
{
	context->set_clear_color(0.5f, 0.7f, 1.0f, 1.0f);
	context->set_clear_depth(1.0f);
	context->begin_renderpass(m_renderPass, info->fb);
	context->set_pipeline(info->meshPipeline);
	scene->render_entities(context, camera, 0);
	//render_scene(context, scene, uniformOffset);

	Ref<Terrain> terrain = scene->get_terrain();
	if (scene->get_terrain())
	{
		context->set_pipeline(info->terrainPipeline);
		terrain->render_no_renderpass(context, camera);
	}

	if(reflectionPass)
	{
		Ref<Skybox> skybox = scene->get_skybox();
		ShaderBindings* bindings = skybox->get_cubemap_bindings();
		context->update_pipeline(m_offscreenCubemapPipeline, &bindings, 1);
		context->set_pipeline(m_offscreenCubemapPipeline);

		struct UniformData
		{
			glm::mat4 projection;
			glm::mat4 view;
			vec4 cameraPosition;
			vec4 lightDirection;
		} uniformData;
		uniformData.projection = camera->get_projection();
		uniformData.view = camera->get_view();

		Ref<Mesh> cubeMesh = scene->get_cube_mesh();
		context->set_uniform(ShaderStage::Vertex, 0, sizeof(uniformData), &uniformData);
		VertexBufferView* vb = cubeMesh->get_vb();
		context->set_buffer(vb->buffer, vb->offset);
		IndexBufferView* ib = cubeMesh->get_ib();
		context->set_buffer(ib->buffer, ib->offset);
		context->draw_indexed(cubeMesh->get_indices_count());
	}
	context->end_renderpass();
}
void Water::destroy()
{
	m_butterflyTexture->destroy();
	m_spectrumTexture->destroy();
	m_fft->destroy();
	m_inversion->destroy();
	m_renderer->destroy();
	m_normaMapGenerator->destroy();

	Device::destroy_shader_bindings(m_rendererBindings);
	Device::destroy_shader_bindings(m_fftBindings);
	Device::destroy_shader_bindings(debugBindings);
	Device::destroy_shader_bindings(m_reflection.binding);
	Device::destroy_shader_bindings(m_refraction.binding);

	Device::destroy_pipeline(m_reflection.meshPipeline);
	Device::destroy_pipeline(m_refraction.meshPipeline);

	Device::destroy_pipeline(m_reflection.terrainPipeline);
	Device::destroy_pipeline(m_refraction.terrainPipeline);
	Device::destroy_pipeline(m_offscreenCubemapPipeline);

	Device::destroy_renderpass(m_renderPass);
	Device::destroy_framebuffer(m_reflection.fb);
	Device::destroy_framebuffer(m_refraction.fb);

	Device::destroy_buffer(m_waterUniformParams);
	Device::destroy_buffer(m_reflection.ubo);
	Device::destroy_buffer(m_refraction.ubo);
}

