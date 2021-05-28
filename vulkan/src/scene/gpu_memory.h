#pragma once

#include <stdint.h>
#include <vector>
#include "renderer/buffer.h"
#include "renderer/device.h"

namespace GPU_MEMORY_POOL
{
	class GpuMemory
	{
	public:
		static GpuMemory* get_instance()
		{
			static GpuMemory* memory = new GpuMemory();
			return memory;
		}
		// static buffer only for now
		VertexBufferView* allocate_vb(uint32_t size)
		{
			if (activeVB.buffer == nullptr || (size > activeVB.size - activeVB.offset))
			{
				uint32_t allocationSize = size > defaultAllocationSize ? size : defaultAllocationSize;
				activeVB.buffer = Device::create_vertexbuffer(BufferUsageHint::StaticDraw, allocationSize);
				activeVB.size = allocationSize;
				activeVB.offset = size;
				allocatedVB.push_back(activeVB);
				return new VertexBufferView{ activeVB.buffer, size, 0 };
			}
			else
			{
				uint32_t offset = activeVB.offset;
				activeVB.offset += size;
				return new VertexBufferView{ activeVB.buffer, size, offset };
			}
		}

		IndexBufferView* allocate_ib(uint32_t size)
		{
			if (activeIB.buffer == nullptr || (size > activeIB.size - activeIB.offset))
			{
				uint32_t allocationSize = size > defaultAllocationSize ? size : defaultAllocationSize;
				activeIB.buffer = Device::create_indexbuffer(BufferUsageHint::StaticDraw, IndexType::UnsignedInt, allocationSize);
				activeIB.size = allocationSize;
				activeIB.offset = size;
				allocatedIB.push_back(activeIB);
				return new IndexBufferView{ activeIB.buffer, size, 0 };
			}
			else
			{
				uint32_t offset = activeIB.offset;
				activeIB.offset += size;
				return new IndexBufferView{ activeIB.buffer, size, offset };
			}
		}

		void destroy()
		{
			for (auto& buffer : allocatedVB)
				Device::destroy_buffer(buffer.buffer);
			for (auto& buffer : allocatedIB)
				Device::destroy_buffer(buffer.buffer);

		}
	private:
		struct VertexBufferInfo
		{

			VertexBuffer* buffer;
			uint32_t size;
			uint32_t offset;
		};

		struct IndexBufferInfo
		{

			IndexBuffer* buffer;
			uint32_t size;
			uint32_t offset;
		};

		VertexBufferInfo activeVB = {};
		IndexBufferInfo activeIB = {};
		std::vector<VertexBufferInfo> allocatedVB;
		std::vector<IndexBufferInfo> allocatedIB;

		// default allocation size 10MB
		const uint32_t defaultAllocationSize = 1024 * 1024 * 10;
	};
};