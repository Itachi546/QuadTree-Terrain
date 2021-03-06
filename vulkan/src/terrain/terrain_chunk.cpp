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

	const float transitionRegion = 200.0f;
	float hW  = float(stream->get_width()) * 0.5f;
	float hH = float(stream->get_height()) * 0.5f;

	float radius = glm::max(hW, hH) - transitionRegion;
	float distance = glm::length(vec2(x - hW, y - hH)) - radius;
	if (distance < 0.0f)
		return  h * maxHeight;
	float factor = glm::max(distance, 0.0f) / transitionRegion;
	factor = glm::smoothstep(0.0f, 1.0f, factor);
	return glm::lerp(h * maxHeight, -maxHeight, factor);
}

uint32_t compress_normal(const glm::vec3& normal)
{
	glm::vec3 n = normal * 0.5f + 0.5f;
	uint8_t x = uint8_t(n.x * 255);
	uint8_t y = uint8_t(n.y * 255);
	uint8_t z = uint8_t(n.z * 255);

	uint32_t result = 0;
	result = (result | x) << 8;
	result = (result | y) << 8;
	result = result | z;

	return result;
}

void TerrainChunk::create_mesh(Ref<TerrainStream> stream, const ivec3& terrainSize, uint32_t vertexCount, std::vector<VertexP4N1_Float>& vertices)
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
			
			float nextHeight = h;
			if (m_lodLevel > 0)
			{
				int scale = static_cast<int>(std::pow(2.0f, m_lodLevel));
				glm::vec2 modPos = glm::vec2{ glm::mod(float(ix / scale), 2.0f), glm::mod(float(iz / scale), 2.0f) };
				if (glm::length(glm::vec2(modPos)) > 0.5f)
				{
					float h1 = get_height(stream, ix + modPos.x, iz + modPos.y, maxHeight);
					float h2 = get_height(stream, ix - modPos.x, iz - modPos.y, maxHeight);
					nextHeight = (h1 + h2) * 0.5f;
				}
			}

			VertexP4N1_Float vertex;
			vertex.position = glm::vec4(fx, h, fz, nextHeight);
			vertex.normal = compress_normal(glm::normalize(glm::vec3(a - b, 1.0f, d - c)));

			int index = (z + 1) * (VERTEX_COUNT + 3) + (x + 1);
			vertices[index] = vertex;
		}
	}

	// Displace the skirt
	if (m_lodLevel > 0)
	{
		int z = 0;
		const float skirtHeight = 0.0f;
		for (int x = -1; x <= VERTEX_COUNT + 1; ++x)
		{
			int index = (z + 1) * (VERTEX_COUNT + 3) + (x + 1);
			VertexP4N1_Float& vertex = vertices[index];
			vertex.position.y -= skirtHeight;
		}

		z = VERTEX_COUNT + 1;
		for (int x = -1; x <= VERTEX_COUNT + 1; ++x)
		{
			int index = (z + 1) * (VERTEX_COUNT + 3) + (x + 1);
			VertexP4N1_Float& vertex = vertices[index];
			vertex.position.y -= skirtHeight;
		}

		int x = 0;
		for (int z = -1; z <= VERTEX_COUNT + 1; ++z)
		{
			int index = (z + 1) * (VERTEX_COUNT + 3) + (x + 1);
			VertexP4N1_Float& vertex = vertices[index];
			vertex.position.y -= skirtHeight;
		}

		x = VERTEX_COUNT + 1;
		for (int z = -1; z <= VERTEX_COUNT + 1; ++z)
		{
			int index = (z + 1) * (VERTEX_COUNT + 3) + (x + 1);
			VertexP4N1_Float& vertex = vertices[index];
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
	std::vector<VertexP4N1_Float> vertices;
	create_mesh(stream, terrainSize, vertexCount, vertices);
	//m_mesh->finalize(context);
	ASSERT(vertices.size() * sizeof(VertexP4N1_Float) == vb->size);
	context->copy(vb->buffer, vertices.data(), vb->offset, vb->size);
	m_loaded = true;
}
