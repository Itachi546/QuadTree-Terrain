#include "mesh.h"
#include "gpu_memory.h"
#include "renderer/context.h"

using namespace GPU_MEMORY_POOL;
void Mesh::finalize(Context* context)
{
	GpuMemory* memory = GpuMemory::get_instance();
	vb = memory->allocate_vb(sizeof(Vertex) * static_cast<uint32_t>(vertices.size()));
	context->copy(vb->buffer, vertices.data(), vb->offset, vb->size);
	ib = memory->allocate_ib(sizeof(uint32_t) * static_cast<uint32_t>(indices.size()));
	context->copy(ib->buffer, indices.data(), ib->offset, ib->size);

	indices_count = static_cast<uint32_t>(indices.size());
	// @TODO is it needed later?
	vertices.clear();
	indices.clear();
}
