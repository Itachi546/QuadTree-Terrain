#include "cascaded_shadow.h"
#include "scene/camera.h"

#include "renderer/framebuffer.h"
#include "renderer/renderpass.h"
#include "renderer/pipeline.h"
#include "renderer/device.h"
#include "renderer/context.h"
#include "common/common.h"
#include "scene/scene.h"
#include "renderer/shaderbinding.h"
#include "renderer/buffer.h"
#include "scene/entity.h"

ShadowCascade::ShadowCascade(const glm::vec3& direction) : m_direction(direction)
{
	{
		RenderPassDescription desc = {};
		desc.attachmentCount = 1;
		Attachment attachment = {0, Format::D32Float, TextureType::DepthStencil};
		desc.attachments = &attachment;
		desc.width = SHADOW_MAP_DIMENSION;
		desc.height = SHADOW_MAP_DIMENSION;
		m_renderPass = Device::create_renderpass(desc);
	}

	{
		PipelineDescription desc = {};
		std::string vertexCode = load_file("spirv/shadow.vert.spv");
		ASSERT(vertexCode.size() % 4 == 0);
		std::string fragmentCode = load_file("spirv/shadow.frag.spv");
		ASSERT(fragmentCode.size() % 4 == 0);
		ShaderDescription shaderDescription[2] = {
			ShaderDescription{ShaderStage::Vertex, vertexCode, static_cast<uint32_t>(vertexCode.size())},
			ShaderDescription{ShaderStage::Fragment, fragmentCode, static_cast<uint32_t>(fragmentCode.size())}
		};
		desc.shaderStageCount = 2;
		desc.shaderStages = shaderDescription;
		desc.renderPass = m_renderPass;
		desc.rasterizationState.enableDepthTest = true;
		desc.rasterizationState.enableDepthWrite = true;
		desc.rasterizationState.faceCulling = FaceCulling::None;
		m_pipeline = Device::create_pipeline(desc);
	}

	{
		TextureDescription desc = {};
		desc.width = SHADOW_MAP_DIMENSION;
		desc.height = SHADOW_MAP_DIMENSION;
		desc.format = Format::D32Float;
		desc.type = TextureType::DepthStencil;

		SamplerDescription sampler = SamplerDescription::Initialize();
		desc.sampler = &sampler;

		FramebufferDescription fbDesc = {};
		fbDesc.attachmentCount = 1;
		fbDesc.attachments = &desc;
		fbDesc.width = SHADOW_MAP_DIMENSION;
		fbDesc.height = SHADOW_MAP_DIMENSION;

		for (int i = 0; i < CASCADE_COUNT; ++i)
			m_cascadeFramebuffer[i] = Device::create_framebuffer(fbDesc, m_renderPass);
	}

	// Create uniform buffer
	m_ubo = Device::create_uniformbuffer(BufferUsageHint::DynamicRead, CASCADE_COUNT * sizeof(Cascade));
	m_bindings = Device::create_shader_bindings();
	for (int i = 0; i < CASCADE_COUNT; ++i)
	{
		m_bindings->set_texture(m_cascadeFramebuffer[i]->get_depth_attachment(), i + 2);
	}
	m_bindings->set_buffer(m_ubo,  2 + CASCADE_COUNT);


}

void ShadowCascade::update(Ref<Camera> camera)
{
	float nearClip = camera->get_near_plane();
	float farClip = camera->get_far_plane();
	float clipRange = farClip - nearClip;

	float minZ = nearClip;
	float maxZ = nearClip + clipRange;

	float range = maxZ - minZ;
	float ratio = maxZ / minZ;

	for (uint32_t i = 0; i < CASCADE_COUNT; i++) {
		float p = (i + 1) / static_cast<float>(CASCADE_COUNT);
		float log = minZ * std::pow(ratio, p);
		float uniform = minZ + range * p;
		float d = cascadeSplitLambda * (log - uniform) + uniform;
		m_cascadeSplits[i] = (d - nearClip) / clipRange;
	}

	// Calculate orthographic projection matrix for each cascade
	const std::array<glm::vec3, 8> cameraFrustum = camera->get_frustum_corner();
	float lastSplitDist = 0.0;
	for (uint32_t i = 0; i < CASCADE_COUNT; i++) {
		float splitDist = m_cascadeSplits[i];
		glm::vec3 frustumCorners[8] = {};
		for (uint32_t i = 0; i < 4; i++) {
			glm::vec3 dist = cameraFrustum[i + 4] - cameraFrustum[i];
			frustumCorners[i + 4] = cameraFrustum[i] + (dist * splitDist);
			frustumCorners[i] = cameraFrustum[i] + (dist * lastSplitDist);
		}

		// Get frustum center
		glm::vec3 frustumCenter = glm::vec3(0.0f);
		for (uint32_t i = 0; i < 8; i++)
			frustumCenter += frustumCorners[i];
		frustumCenter /= 8.0f;

		float radius = 0.0f;
		for (uint32_t i = 0; i < 8; i++) 
		{
			float distance = glm::length2(frustumCorners[i] - frustumCenter);
			radius = glm::max(radius, distance);
		}
		radius = std::sqrt(radius);
		radius = std::ceil(radius * 16.0f) / 16.0f;

		// Either calculate orthographic projection by transforming the 
		// view frustum or calculate view matrix
		glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter + m_direction * radius, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 lightOrthoMatrix = glm::ortho(-radius, radius, -radius, radius, 0.0f, 2.0f * radius);

		// Store split distance and matrix in cascade
		m_cascades[i].splitDepth = glm::vec4((camera->get_near_plane() + splitDist * clipRange));
		m_cascades[i].VP = lightOrthoMatrix * lightViewMatrix;
		lastSplitDist = m_cascadeSplits[i];
	}
}

void ShadowCascade::render(Context* context, Scene* scene)
{
	context->copy(m_ubo, m_cascades.data(), 0, sizeof(Cascade) * CASCADE_COUNT);
	context->set_graphics_pipeline(m_pipeline);
	
	EntityIterator begin, end;
	scene->get_entity_iterator(begin, end);
	for (int i = 0; i < CASCADE_COUNT; ++i)
	{
		context->set_clear_depth();
		context->begin_renderpass(m_renderPass, m_cascadeFramebuffer[i]);
		context->set_graphics_pipeline(m_pipeline);
		context->set_uniform(ShaderStage::Vertex, sizeof(glm::mat4), sizeof(glm::mat4), &m_cascades[i].VP[0][0]);

		for (EntityIterator entity = begin; entity != end; entity++)
		{
			glm::mat4 model = (*entity)->transform->get_mat4();
			Ref<Mesh> mesh = (*entity)->mesh;
			VertexBufferView* vb = mesh->get_vb();
			IndexBufferView* ib = mesh->get_ib();
			context->set_buffer(vb->buffer, vb->offset);
			context->set_buffer(ib->buffer, ib->offset);
			context->set_uniform(ShaderStage::Vertex, 0, sizeof(mat4), &model[0][0]);
			context->draw_indexed(mesh->get_indices_count());
		}
		context->end_renderpass();
	}
}

void ShadowCascade::destroy()
{
	Device::destroy_renderpass(m_renderPass);
	Device::destroy_pipeline(m_pipeline);
	Device::destroy_buffer(m_ubo);
	for (int i = 0; i < CASCADE_COUNT; ++i)
		Device::destroy_framebuffer(m_cascadeFramebuffer[i]);
}
