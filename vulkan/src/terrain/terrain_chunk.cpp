#include "terrain_chunk.h"
#include "terrain_stream.h"
#include "renderer/context.h"
#include "renderer/device.h"
#include "renderer/buffer.h"

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

void TerrainChunk::create_mesh(Ref<TerrainStream> stream, const ivec3& terrainSize, uint32_t vertexCount, std::vector<VertexP4N3_Float>& vertices)
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

	vertices.resize((VERTEX_COUNT + 3) * (VERTEX_COUNT + 3));
	for (int z = -1; z <= VERTEX_COUNT + 1; ++z)
	{
		float fz = float(z) / float(VERTEX_COUNT);
		fz = (m_min.y + fz * rangeZ);
		for (int x = -1; x <= VERTEX_COUNT + 1; ++x)
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

			VertexP4N3_Float vertex;
			vertex.position = glm::vec4(fx, h, fz, 1.0f);
			vertex.normal = glm::normalize(glm::vec3(a - b, 1.0f, d - c));

			int index = (z + 1) * (VERTEX_COUNT + 3) + (x + 1);
			vertices[index] = vertex;
		}
	}
	// Displace the skirt
	if (m_lodLevel > 0)
	{
		int z = 0;
		const float skirtHeight = 1.0f;
		for (int x = -1; x <= VERTEX_COUNT + 1; ++x)
		{
			int index = (z + 1) * (VERTEX_COUNT + 3) + (x + 1);
			VertexP4N3_Float& vertex = vertices[index];
			vertex.position.y -= skirtHeight;
		}

		z = VERTEX_COUNT + 1;
		for (int x = -1; x <= VERTEX_COUNT + 1; ++x)
		{
			int index = (z + 1) * (VERTEX_COUNT + 3) + (x + 1);
			VertexP4N3_Float& vertex = vertices[index];
			vertex.position.y -= skirtHeight;
		}

		int x = 0;
		for (int z = -1; z <= VERTEX_COUNT + 1; ++z)
		{
			int index = (z + 1) * (VERTEX_COUNT + 3) + (x + 1);
			VertexP4N3_Float& vertex = vertices[index];
			vertex.position.y -= skirtHeight;
		}

		x = VERTEX_COUNT + 1;
		for (int z = -1; z <= VERTEX_COUNT + 1; ++z)
		{
			int index = (z + 1) * (VERTEX_COUNT + 3) + (x + 1);
			VertexP4N3_Float& vertex = vertices[index];
			vertex.position.y -= skirtHeight;
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
	std::vector<VertexP4N3_Float> vertices;
	create_mesh(stream, terrainSize, vertexCount, vertices);
	//m_mesh->finalize(context);
	ASSERT(vertices.size() * sizeof(VertexP4N3_Float) == vb->size);
	context->copy(vb->buffer, vertices.data(), vb->offset, vb->size);
	m_loaded = true;
}
