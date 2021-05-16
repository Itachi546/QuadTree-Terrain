#include "terrain.h"
#include "terrain_stream.h"

#include "renderer/buffer.h"
#include "renderer/pipeline.h"
#include "renderer/device.h"
#include "scene/mesh.h"
#include "common/common.h"
#include "renderer/context.h"


float Terrain::get_height(int x, int y)
{
	const float heightScale = 120.0f;
	return ((static_cast<float>(m_stream->get(x + 1, y + 1)) / 255.0f) * 2.0f - 1.0f)* heightScale;
}

Terrain::Terrain(Context* context, Ref<TerrainStream> stream) : m_stream(stream)
{
	std::string vertexCode = load_file("spirv/terrain.vert.spv");
	ASSERT(vertexCode.size() % 4 == 0);
	std::string fragmentCode = load_file("spirv/terrain.frag.spv");
	ASSERT(fragmentCode.size() % 4 == 0);

	PipelineDescription desc = {};
	ShaderDescription shaderDescription[2] = {};
	shaderDescription[0].shaderStage = ShaderStage::Vertex;
	shaderDescription[0].code = vertexCode;
	shaderDescription[0].sizeInByte = static_cast<uint32_t>(vertexCode.size());
	shaderDescription[1].shaderStage = ShaderStage::Fragment;
	shaderDescription[1].code = fragmentCode;
	shaderDescription[1].sizeInByte = static_cast<uint32_t>(fragmentCode.size());
	desc.shaderStageCount = 2;
	desc.shaderStages = shaderDescription;
	desc.renderPass = context->get_global_renderpass();
	desc.rasterizationState.enableDepthTest = true;
	desc.rasterizationState.faceCulling = FaceCulling::None;
	desc.rasterizationState.polygonMode = PolygonMode::Fill;
	m_pipeline = Device::create_pipeline(desc);

	const double frequency = 0.05;
	noise = new siv::PerlinNoise(1024);
	m_mesh = CreateRef<Mesh>();

	const float scalingFactor = 0.1f;
	int w = stream->get_width() / 8;
	int h = stream->get_height() / 8;

	std::vector<Vertex> vertices;
	for (int z = 0; z <= h; ++z)
	{
		for (int x = 0; x <= w; ++x)
		{
			float h = get_height(x, z);
			float a = get_height(x + 1, z);
			float b = get_height(x - 1, z);
			float c = get_height(x, z + 1);
			float d = get_height(x, z - 1);

			h = (h + a + b + c + d) / 5.0f;

			Vertex vertex;
			vertex.position = glm::vec3(x - w * 0.5f, h, z - h * 0.5f);
			vertex.normal = glm::normalize(glm::vec3(a - b, 1.0f, d - c));
			vertices.push_back(vertex);
		}
	}

	/*************/
	/*i0*****i1**/
	/*i2*****i3**/
	/*************/
	std::vector<unsigned int> indices;
	for (int z = 0; z < h; ++z)
	{
		for (int x = 0; x < w; ++x)
		{
			uint32_t i0 = z * (w + 1) + x;
			uint32_t i1 = i0 + 1;
			uint32_t i2 = i0 + (w + 1);
			uint32_t i3 = i2 + 1;

			glm::vec3 n0 = vertices[i0].normal;
			glm::vec3 n1 = vertices[i1].normal;
			glm::vec3 n2 = vertices[i2].normal;
			glm::vec3 n3 = vertices[i3].normal;

			glm::vec3 new_n1 = (n0 + n1 + n2) / 3.0f;
			glm::vec3 new_n2 = (n1 + n2 + n3) / 3.0f;

			vertices[i0].normal = glm::normalize(new_n1);
			vertices[i1].normal = glm::normalize((new_n1 + new_n2) * 0.5f);
			vertices[i2].normal = glm::normalize((new_n1 + new_n2) * 0.5f);
			vertices[i3].normal = glm::normalize(new_n2);

			indices.push_back(i2);
			indices.push_back(i1);
			indices.push_back(i0);

			indices.push_back(i3);
			indices.push_back(i2);
			indices.push_back(i1);
		}
	}
	m_mesh->vertices = vertices;
	m_mesh->indices = indices;

	m_mesh->finalize(context);
}

void Terrain::render(Context* context, ShaderBindings* uniformBindings)
{
	context->set_graphics_pipeline(m_pipeline);
	ShaderBindings* bindingArr[] = { uniformBindings };
	context->set_shader_bindings(bindingArr, 1);
	glm::mat4 model = glm::mat4(1.0f);
	context->set_uniform(ShaderStage::Vertex, 0, sizeof(glm::mat4), &model[0][0]);
	VertexBufferView* vb = m_mesh->get_vb();
	context->set_buffer(vb->buffer, vb->offset);

	IndexBufferView* ib = m_mesh->get_ib();
	context->set_buffer(ib->buffer, ib->offset);
	context->draw_indexed(m_mesh->indices_count);
}

void Terrain::destroy()
{
	Device::destroy_pipeline(m_pipeline);
	if(m_stream)
		m_stream->destroy();
}
