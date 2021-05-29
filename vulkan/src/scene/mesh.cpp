#include "mesh.h"
#include "gpu_memory.h"
#include "renderer/context.h"

using namespace GPU_MEMORY_POOL;

void Mesh::calculate_bounding_box()
{
	glm::vec3 min = glm::vec3( FLT_MAX);
	glm::vec3 max = glm::vec3(-FLT_MAX);

	for (const auto& vertex : vertices)
	{
		min.x = glm::min(vertex.position.x, min.x);
		min.y = glm::min(vertex.position.y, min.y);
		min.z = glm::min(vertex.position.z, min.z);

		max.x = glm::max(vertex.position.x, max.x);
		max.y = glm::max(vertex.position.y, max.y);
		max.z = glm::max(vertex.position.z, max.z);
	}

	boundingBox.min = min;
	boundingBox.max = max;
}

void Mesh::finalize(Context* context)
{
	if (vb == nullptr || ib == nullptr)
	{
		GpuMemory* memory = GpuMemory::get_instance();
		vb = memory->allocate_vb(sizeof(Vertex) * static_cast<uint32_t>(vertices.size()));
		ib = memory->allocate_ib(sizeof(uint32_t) * static_cast<uint32_t>(indices.size()));
	}

	context->copy(vb->buffer, vertices.data(), vb->offset, vb->size);
	context->copy(ib->buffer, indices.data(), ib->offset, ib->size);
	indices_count = static_cast<uint32_t>(indices.size());
	// @TODO is it needed later?
	vertices.clear();
	indices.clear();
}
