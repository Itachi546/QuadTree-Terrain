#include "water_renderer.h"
#include "renderer/pipeline.h"
#include "common/common.h"
#include "renderer/context.h"
#include "renderer/device.h"
#include "renderer/shaderbinding.h"
#include "renderer/buffer.h"
#include "core/math.h"

#include <vector>

#define USE_DISPLACED_WATER 0

void create_grid_mesh(std::vector<glm::vec3>& vertices, std::vector<uint32_t>& indices, uint32_t width, uint32_t height)
{
#if USE_DISPLACED_WATER
	// Terrain Width and height
	int VERTEX_COUNT = width;
	float m = 1.0f / float(VERTEX_COUNT);

	for (int z = 0; z < VERTEX_COUNT; ++z)
	{
		float fz = z * m * height - height * 0.5f;
		
		for (int x = 0; x < VERTEX_COUNT; ++x)
		{
			float fx = x * m * width - width * 0.5f;
			vertices.emplace_back(fx, 0.0f, fz);
		}
	}

	for (int z = 0; z < VERTEX_COUNT - 1; ++z)
	{
		for (int x = 0; x < VERTEX_COUNT - 1; ++x)
		{
			uint32_t i0 = z * VERTEX_COUNT + x;
			uint32_t i1 = i0 + 1;
			uint32_t i2 = i0 + VERTEX_COUNT;
			uint32_t i3 = i2 + 1;

			indices.push_back(i2);
			indices.push_back(i1);
			indices.push_back(i0);

			indices.push_back(i2);
			indices.push_back(i3);
			indices.push_back(i1);
		}
	}
#else
	float hW = width * 0.5f;
	float hH = height * 0.5f;
	vertices.emplace_back(-hW, 0.0f, -hH);
	vertices.emplace_back(-hW, 0.0f,  hH);
	vertices.emplace_back( hW, 0.0f,  hH);
	vertices.emplace_back( hW, 0.0f, -hH);

	indices = {
		2, 0, 1,
		0, 2, 3
	};
#endif
}
	

WaterRenderer::WaterRenderer(Context* context)
{
	{
#if USE_DISPLACED_WATER
		std::string vertexCode = load_file("spirv/water.vert.spv");
		ASSERT(vertexCode.size() % 4 == 0);
		std::string fragmentCode = load_file("spirv/water.frag.spv");
		ASSERT(fragmentCode.size() % 4 == 0);
#else
		std::string vertexCode = load_file("spirv/water_nodisp.vert.spv");
		ASSERT(vertexCode.size() % 4 == 0);
		std::string fragmentCode = load_file("spirv/water_nodisp.frag.spv");
		ASSERT(fragmentCode.size() % 4 == 0);
#endif
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
		pipelineDesc.rasterizationState.polygonMode = PolygonMode::Fill;
		pipelineDesc.rasterizationState.topology = Topology::Triangle;
		pipelineDesc.blendState.enable = true;
		m_pipeline = Device::create_pipeline(pipelineDesc);
	}

	std::vector<glm::vec3> vertices;
	std::vector<uint32_t> indices;
	create_grid_mesh(vertices, indices, 4096, 4096);

	uint32_t sizeofVertexData = static_cast<uint32_t>(vertices.size()) * sizeof(glm::vec3);
	m_vbo = Device::create_vertexbuffer(BufferUsageHint::StaticDraw, sizeofVertexData);
	context->copy(m_vbo, vertices.data(), 0, sizeofVertexData);

	uint32_t sizeofIndexData = static_cast<uint32_t>(indices.size()) * sizeof(uint32_t);
	m_ibo = Device::create_indexbuffer(BufferUsageHint::StaticDraw, IndexType::UnsignedInt, sizeofIndexData);
	context->copy(m_ibo, indices.data(), 0, sizeofIndexData);
	m_indicesCount = static_cast<uint32_t>(indices.size());
}

void WaterRenderer::render(Context* context, ShaderBindings** uniformBindings, glm::vec3 cameraPos, glm::vec3 translate, uint32_t count)
{
	context->update_pipeline(m_pipeline, uniformBindings, count);
	context->set_pipeline(m_pipeline);
	context->set_uniform(ShaderStage::Vertex, 0, sizeof(glm::vec4), &translate);
	context->set_uniform(ShaderStage::Vertex, sizeof(glm::vec4), sizeof(glm::vec3), &cameraPos);

	context->set_buffer(m_vbo, 0);
	context->set_buffer(m_ibo, 0);
	context->draw_indexed(m_indicesCount);
}

void WaterRenderer::destroy()
{
	Device::destroy_buffer(m_vbo);
	Device::destroy_buffer(m_ibo);
	Device::destroy_pipeline(m_pipeline);
}
