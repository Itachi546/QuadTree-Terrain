#include "terrain_chunk.h"
#include "terrain_stream.h"
#include "scene/mesh.h"
#include "renderer/context.h"

float get_height(Ref<TerrainStream> stream, float x, float y, float maxHeight)
{
	int ix = static_cast<int>(x);
	int iy = static_cast<int>(y);

	float a = stream->get(ix, iy);
	float b = stream->get(ix + 1, iy);
	float c = stream->get(ix, iy + 1);
	float d = stream->get(ix + 1, iy + 1);

	float fx = x - ix;
	float fy = y - iy;

	float h0 = glm::mix(a, b, fx);
	float h1 = glm::mix(c, d, fx);
	
	float h = glm::mix(h0, h1, fy) * 2.0f - 1.0f;
	return  h * maxHeight;
}

void TerrainChunk::create_mesh(Ref<TerrainStream> stream, const ivec3& terrainSize, uint32_t vertexCount, std::vector<Vertex>& vertices)
{
	// Terrain Width and height
	int VERTEX_COUNT = vertexCount;

	uint32_t width = stream->get_width();
	uint32_t height = stream->get_height();

	float mX = 1.0f / float(terrainSize.x);
	float mZ = 1.0f / float(terrainSize.z);

	float rangeZ = float(m_max.y - m_min.y);
	float rangeX = float(m_max.x - m_min.x);
	float maxHeight = float(terrainSize.y);

	for (int z = 0; z <= VERTEX_COUNT; ++z)
	{
		float fz = float(z) / float(VERTEX_COUNT);
		fz = (m_min.y + fz * rangeZ);
		for (int x = 0; x <= VERTEX_COUNT; ++x)
		{
			float fx = float(x) / float(VERTEX_COUNT);
			fx = (m_min.x + fx * rangeX);

			float uvx = fx * mX * (width - 3) + 1.0f;
			float uvz = fz * mZ * (height - 3) + 1.0f;

			int ix = static_cast<int>(uvx);
			int iz = static_cast<int>(uvz);

			float h = get_height(stream, uvx, uvz, maxHeight);
			float a = get_height(stream, uvx + 1.0f, uvz, maxHeight);
			float b = get_height(stream, uvx - 1.0f, uvz, maxHeight);
			float c = get_height(stream, uvx, uvz + 1.0f, maxHeight);
			float d = get_height(stream, uvx, uvz - 1.0f, maxHeight);

			Vertex vertex;
			vertex.position = glm::vec3(fx, h, fz);
			vertex.normal = glm::normalize(glm::vec3(a - b, 1.0f, d - c));
			vertices.push_back(vertex);
		}
	}
}

TerrainChunk::TerrainChunk(Ref<VertexBufferView> vb) : vb(vb)
{
	//m_mesh = new Mesh();
	m_id = UINT32_MAX;
	m_lastFrameIndex = 0;
}

void TerrainChunk::initialize(const glm::ivec2& min, const glm::ivec2& max, uint32_t lod_level, uint32_t id, uint64_t lastFrameIndex)
{
	m_min = min;
	m_max = max;
	m_lodLevel = lod_level;
	m_loaded = false;
	m_id = id;
	m_lastFrameIndex = lastFrameIndex;
}

void TerrainChunk::build(Context* context, Ref<TerrainStream> stream, const glm::ivec3& terrainSize, uint32_t vertexCount)
{
	std::vector<Vertex> vertices;
	create_mesh(stream, terrainSize, vertexCount, vertices);
	//m_mesh->finalize(context);
	ASSERT(vertices.size() * sizeof(Vertex) == vb->size);
	context->copy(vb->buffer, vertices.data(), vb->offset, vb->size);
	m_loaded = true;
}
