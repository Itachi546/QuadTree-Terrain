#include "terrain.h"
#include "terrain_stream.h"
#include "terrain_quadtree.h"
#include "terrain_chunkmanager.h"

#include "renderer/buffer.h"
#include "renderer/pipeline.h"
#include "renderer/device.h"
#include "scene/mesh.h"
#include "common/common.h"
#include "renderer/context.h"
#include "scene/camera.h"

float sample_height(Ref<TerrainStream> stream, int x, int y)
{
	const float heightScale = 480.0f;
	return (stream->get(x, y)) * heightScale;
}

Terrain::Terrain(Context* context, Ref<TerrainStream> stream): m_stream(stream)
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

	uint32_t terrainSize = stream->get_width();;
	uint32_t depth = static_cast<int>(std::log2(terrainSize / minchunkSize));
	m_maxLod = depth;

	m_quadTree = CreateRef<QuadTree>(depth, terrainSize);

	m_chunkManager = CreateRef<TerrainChunkManager>();
	m_chunkManager->init(stream, glm::ivec2(terrainSize));
}

float Terrain::get_height(glm::vec3 position)
{
	int x = static_cast<int>(glm::floor(position.x));
	int y = static_cast<int>(glm::abs(glm::floor(position.z)));

	float h1 = sample_height(m_stream, x, y);
	float a = sample_height(m_stream, x + 1, y);
	float b = sample_height(m_stream, x -1, y);
	float c = sample_height(m_stream, x, y + 1);	
	float d = sample_height(m_stream, x, y - 1);
	return (h1 + a + b + c + d) / 5.0f;
}

void Terrain::update(Context* context, Ref<Camera> camera)
{
	m_quadTree->insert(camera->get_position());
	//m_quadTree->insert(glm::vec3(0.0f));
	//m_quadTree->debug();
	m_chunkManager->update(context, camera, m_quadTree->leafNodes, m_maxLod);
}

void Terrain::update(glm::vec3 position)
{
	m_quadTree->insert(position);
	//m_quadTree->debug();
}

void Terrain::render(Context* context, ShaderBindings** uniformBindings, int count)
{
	context->set_graphics_pipeline(m_pipeline);
	context->set_shader_bindings(uniformBindings, count);

	glm::mat4 model = glm::mat4(1.0f);
	context->set_uniform(ShaderStage::Vertex, 0, sizeof(glm::mat4), &model[0][0]);

	m_chunkManager->render(context);
}

void Terrain::destroy()
{
	Device::destroy_pipeline(m_pipeline);
}
