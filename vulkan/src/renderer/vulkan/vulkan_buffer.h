#pragma once

#include "renderer/buffer.h"
#include "vulkan_common.h"
#include <stdint.h>

class VulkanAPI;
class VulkanBuffer
{
public:
	VulkanBuffer(std::shared_ptr<VulkanAPI> api, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags, uint32_t sizeInByte);
	void copy(void* data, uint32_t offsetInByte, uint32_t sizeInByte);
	void copy(std::shared_ptr<VulkanAPI> api, VkCommandBuffer commandBuffer, void* data, uint32_t offsetInByte, uint32_t sizeInByte);

	void destroy(std::shared_ptr<VulkanAPI> api);
	VkDeviceMemory memory;
	void* pointer;
	VkBuffer buffer;
	uint32_t size;
};

class VulkanVertexBuffer : public VertexBuffer
{
public:
	VulkanVertexBuffer(std::shared_ptr<VulkanAPI> api, BufferUsageHint usage, uint32_t sizeInByte);

	BufferUsageHint get_usage_hint() const override
	{
		return m_usage;
	}
	int get_size() const override
	{
		return m_buffer->size;
	}

	VkBuffer get_buffer() { return m_buffer->buffer; }
	void copy(std::shared_ptr<VulkanAPI> api, VkCommandBuffer commandBuffer, void* data, uint32_t offsetInByte, uint32_t sizeInByte);

	void destroy(std::shared_ptr<VulkanAPI> api);

private:
	BufferUsageHint m_usage;
	std::shared_ptr<VulkanBuffer> m_buffer;
};


class VulkanIndexBuffer : public IndexBuffer
{
public:
	VulkanIndexBuffer(std::shared_ptr<VulkanAPI> api, IndexType indexType, BufferUsageHint usage, uint32_t sizeInByte);

	BufferUsageHint get_usage_hint() const override
	{
		return m_usage;
	}

	int get_size() const override
	{
		return m_buffer->size;
	}

	VkBuffer get_buffer()
	{
		return m_buffer->buffer;
	}

	VkIndexType get_index_type(){ return m_indexType; }

	void copy(std::shared_ptr<VulkanAPI> api, VkCommandBuffer commandBuffer, void* data, uint32_t offsetInByte, uint32_t sizeInByte);
	void destroy(std::shared_ptr<VulkanAPI> api);

private:
	VkIndexType m_indexType;
	BufferUsageHint m_usage;
	std::shared_ptr<VulkanBuffer> m_buffer;
};

class VulkanUniformBuffer : public UniformBuffer
{
public:
	VulkanUniformBuffer(std::shared_ptr<VulkanAPI> api, BufferUsageHint usage, uint32_t sizeInByte);

	BufferUsageHint get_usage_hint() const override
	{
		return m_usage;
	}

	int get_size() const override
	{
		return m_buffer->size;
	}

	VkBuffer get_buffer()
	{
		return m_buffer->buffer;
	}

	VkDescriptorBufferInfo* get_buffer_info() { return &m_bufferInfo; }

	void copy(std::shared_ptr<VulkanAPI> api, VkCommandBuffer commandBuffer, void* data, uint32_t offsetInByte, uint32_t sizeInByte);
	void destroy(std::shared_ptr<VulkanAPI> api);

private:
	BufferUsageHint m_usage;
	std::shared_ptr<VulkanBuffer> m_buffer;
	VkDescriptorBufferInfo m_bufferInfo;
};


class VulkanShaderStorageBuffer : public ShaderStorageBuffer
{
public:
	VulkanShaderStorageBuffer(std::shared_ptr<VulkanAPI> api, BufferUsageHint usage, uint32_t sizeInByte);

	BufferUsageHint get_usage_hint() const override
	{
		return m_usage;
	}

	int get_size() const override
	{
		return m_buffer->size;
	}

	VkBuffer get_buffer()
	{
		return m_buffer->buffer;
	}

	VkDescriptorBufferInfo* get_buffer_info() { return &m_bufferInfo; }

	void copy(std::shared_ptr<VulkanAPI> api, VkCommandBuffer commandBuffer, void* data, uint32_t offsetInByte, uint32_t sizeInByte);
	void destroy(std::shared_ptr<VulkanAPI> api);

private:
	BufferUsageHint m_usage;
	std::shared_ptr<VulkanBuffer> m_buffer;
	VkDescriptorBufferInfo m_bufferInfo;
};

class VulkanIndirectBuffer : public IndirectBuffer
{
public:
	VulkanIndirectBuffer(std::shared_ptr<VulkanAPI> api, BufferUsageHint usage, uint32_t sizeInByte);
	
	BufferUsageHint get_usage_hint() const override
	{
		return m_usage;
	}

	int get_size() const override
	{
		return m_buffer->size;
	}

	VkBuffer get_buffer()
	{
		return m_buffer->buffer;
	}

	VkDescriptorBufferInfo* get_buffer_info() { return &m_bufferInfo; }

	void copy(std::shared_ptr<VulkanAPI> api, VkCommandBuffer commandBuffer, void* data, uint32_t offsetInByte, uint32_t sizeInByte);
	void destroy(std::shared_ptr<VulkanAPI> api);

private:
	BufferUsageHint m_usage;
	std::shared_ptr<VulkanBuffer> m_buffer;
	VkDescriptorBufferInfo m_bufferInfo;
};