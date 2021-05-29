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
#include "terrain/terrain.h"
#include "core/frustum.h"

#include "imgui/imgui.h"

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
		sampler.minFilter = TextureFilter::Nearest;
		sampler.magFilter = TextureFilter::Nearest;
		sampler.wrapU = WrapMode::ClampToBorder;
		sampler.wrapV = WrapMode::ClampToBorder;
		sampler.wrapW = WrapMode::ClampToBorder;
		desc.sampler = &sampler;

		FramebufferDescription fbDesc = {};
		fbDesc.attachmentCount = 1;
		fbDesc.attachments = &desc;
		fbDesc.width = SHADOW_MAP_DIMENSION;
		fbDesc.height = SHADOW_MAP_DIMENSION;

		for (int i = 0; i < CASCADE_COUNT; ++i)
			m_cascadeFramebuffer[i] = Device::create_framebuffer(fbDesc, m_renderPass);

		calculate_split_distance();
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
	// Calculate orthographic projection matrix for each cascade
	float nearClip = nearDistance;
	const std::array<glm::vec3, 8>& cameraFrustum = camera->get_frustum()->get_points();
	float lastSplitDist = 0.0;
	for (uint32_t j = 0; j < CASCADE_COUNT; j++) 
	{
		float splitDist = m_cascadeSplits[j];
		glm::vec3 frustumCorners[8] = {};

		for (uint32_t i = 0; i < 4; i++) {
			glm::vec3 dist = glm::normalize(cameraFrustum[i + 4] - cameraFrustum[i]);
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
		m_cascades[j].splitDepth = glm::vec4(nearClip + splitDist);
		m_cascades[j].VP = lightOrthoMatrix * lightViewMatrix;
		lastSplitDist = m_cascadeSplits[j];
	}
}

void ShadowCascade::render(Context* context, Scene* scene)
{
	render_ui();


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

#if 0
		glm::mat4 model = glm::mat4(1.0f);
		context->set_uniform(ShaderStage::Vertex, 0, sizeof(mat4), &model[0][0]);
		Ref<Terrain> terrain = scene->get_terrain();
		if (terrain)
			terrain->render(context, scene->get_camera(), &m_bindings, 1, true);
#endif
		context->end_renderpass();
		context->transition_layout_for_shader_read(m_cascadeFramebuffer[i]->get_depth_attachment(), true);
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

void ShadowCascade::render_ui()
{
	if (ImGui::CollapsingHeader("Shadow Cascade"))
	{
		bool changed = false;
		changed |= ImGui::SliderFloat("Split Lambda", &cascadeSplitLambda, 0.0f, 1.0f);
		changed |= ImGui::SliderFloat("Shadow Distance", &shadowDistance, 10.0f, 1000.0f);
		changed |= ImGui::SliderFloat("Near Plane", &nearDistance, 0.01f, 5.0f);

		if (changed)
			calculate_split_distance();
	}
}

void ShadowCascade::calculate_split_distance()
{
	float clipRange = shadowDistance - nearDistance;
	float minZ = nearDistance;
	float maxZ = nearDistance + clipRange;

	float range = maxZ - minZ;
	float ratio = maxZ / minZ;

	for (uint32_t i = 0; i < CASCADE_COUNT; i++) {
		float p = (i + 1) / static_cast<float>(CASCADE_COUNT);
		float log = minZ * std::pow(ratio, p);
		float uniform = minZ + range * p;
		float d = cascadeSplitLambda * (log - uniform) + uniform;
		m_cascadeSplits[i] = (d - nearDistance);
	}
}
