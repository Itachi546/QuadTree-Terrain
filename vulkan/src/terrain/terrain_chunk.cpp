#include "terrain_chunk.h"
#include "terrain_stream.h"
#include "scene/mesh.h"
#include "renderer/context.h"

float get_height(Ref<TerrainStream> stream, int x, int y)
{
	const float heightScale = 480.0f;
	return (stream->get(x, y) * 2.0f - 1.0f) * heightScale * 0.5f;
}

void create_mesh(Mesh* mesh, Ref<TerrainStream> stream, const glm::ivec2& min, const glm::ivec2& max, const ivec2& terrainSize, uint32_t lod_level)
{
	// Terrain Width and height

	uint32_t inc = static_cast<uint32_t>(std::pow(2, lod_level));
	std::vector<Vertex> vertices;
	uint32_t width = stream->get_width();
	uint32_t height = stream->get_height();

	float mX = 1.0f / float(terrainSize.x);
	float mZ = 1.0f / float(terrainSize.y);

	for (int z = min.y; z <= max.y; z += inc)
	{
		float fz = float(z) * mZ;
		for (int x = min.x; x <= max.x; x += inc)
		{
			float fx = float(x) * mX;

			int ix = static_cast<int>(fx * (width - 3)) + 1;
			int iz = static_cast<int>(fz * (height - 3)) + 1;

			float h = get_height(stream, ix, iz);
			float a = get_height(stream, ix + 1, iz);
			float b = get_height(stream, ix - 1, iz);
			float c = get_height(stream, ix, iz + 1);
			float d = get_height(stream, ix, iz - 1);

			Vertex vertex;
			vertex.position = glm::vec3(x, h, z);
			vertex.normal = glm::normalize(glm::vec3(a - b, 1.0f, d - c));
			vertices.push_back(vertex);
		}
	}

	/*************/
	/*i0*****i1**/
	/*i2*****i3**/
	/*************/
	uint32_t wx = (max.x - min.x) / inc;
	uint32_t wz = (max.y - min.y) / inc;

	std::vector<unsigned int> indices;
	for (uint32_t z = 0; z < wz; ++z)
	{
		for (uint32_t x = 0; x < wx; ++x)
		{
			uint32_t i0 = z * (wx + 1) + x;
			uint32_t i1 = i0 + 1;
			uint32_t i2 = i0 + (wx + 1);
			uint32_t i3 = i2 + 1;
			/*
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
			*/
			indices.push_back(i2);
			indices.push_back(i1);
			indices.push_back(i0);

			indices.push_back(i3);
			indices.push_back(i2);
			indices.push_back(i1);
		}
	}
	mesh->vertices = vertices;
	mesh->indices = indices;
}

TerrainChunk::TerrainChunk()
{
	m_mesh = new Mesh();
}

void TerrainChunk::initialize(const glm::ivec2& min, const glm::ivec2& max, uint32_t lod_level)
{
	m_min = min;
	m_max = max;
	m_lodLevel = lod_level;
	m_loaded = false;
}

void TerrainChunk::build(Context* context, Ref<TerrainStream> stream, const glm::ivec2& terrainSize)
{
	create_mesh(m_mesh, stream, m_min, m_max, terrainSize, m_lodLevel);
	m_mesh->finalize(context, true);
	m_loaded = true;
}

void TerrainChunk::render(Context* context)
{
	VertexBufferView* vb = m_mesh->get_vb();
	context->set_buffer(vb->buffer, vb->offset);
	IndexBufferView* ib = m_mesh->get_ib();
	context->set_buffer(ib->buffer, ib->offset);
	context->draw_indexed(m_mesh->indices_count);
}

void TerrainChunk::destroy()
{
	delete m_mesh;
}
