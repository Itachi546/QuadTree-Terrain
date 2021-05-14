#include "vulkan_buffer.h"
#include "vulkan_api.h"
#include "vulkan_type_converter.h"

VulkanBuffer::VulkanBuffer(std::shared_ptr<VulkanAPI> api, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryFlags, uint32_t sizeInByte) : size(sizeInByte), buffer(0), memory(0), pointer(nullptr)
{
	VkDevice device = api->m_Device;
	VkBufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	createInfo.size = sizeInByte;
	createInfo.usage = usage;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	
	VK_CHECK(vkCreateBuffer(device, &createInfo, VK_NULL_HANDLE, &buffer));
	
	VkMemoryRequirements requirements = {};
	vkGetBufferMemoryRequirements(device, buffer, &requirements);

	VkMemoryAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	allocateInfo.allocationSize = requirements.size;
	allocateInfo.memoryTypeIndex = select_memory_type(api->m_MemoryProps, requirements.memoryTypeBits, memoryFlags);

	VK_CHECK(vkAllocateMemory(api->m_Device, &allocateInfo, VK_NULL_HANDLE, &memory));
	VK_CHECK(vkBindBufferMemory(device, buffer, memory, 0));

	if (memoryFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
		VK_CHECK(vkMapMemory(device, memory, 0, sizeInByte, 0, &pointer));
}

void VulkanBuffer::copy(std::shared_ptr<VulkanAPI> api, VkCommandBuffer commandBuffer, void* data, uint32_t offsetInByte, uint32_t sizeInByte)
{
	if (pointer == nullptr)
	{
		VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkResetCommandBuffer(commandBuffer, 0);
		VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

		VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		VulkanBuffer stagingBuffer(api, bufferUsage, properties, sizeInByte);
		stagingBuffer.copy(data, 0, sizeInByte);

		VkBufferCopy copyRegion;
		copyRegion.dstOffset = offsetInByte;
		copyRegion.size = sizeInByte;
		copyRegion.srcOffset = 0;
		vkCmdCopyBuffer(commandBuffer, stagingBuffer.buffer, buffer, 1, &copyRegion);

		VK_CHECK(vkEndCommandBuffer(commandBuffer));

		VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		VkQueue queue = api->m_GraphicsQueue;

		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkDeviceWaitIdle(api->m_Device);
		stagingBuffer.destroy(api);

	}
	else
	{
		copy(data, offsetInByte, sizeInByte);
	}
}

void VulkanBuffer::destroy(std::shared_ptr<VulkanAPI> api)
{
	VkDevice device = api->m_Device;
	vkDestroyBuffer(device, buffer, 0);
	vkFreeMemory(device, memory, 0);
	pointer = nullptr;
}

void VulkanBuffer::copy(void* data, uint32_t offsetInByte, uint32_t sizeInByte)
{
	ASSERT_MSG((offsetInByte + sizeInByte) <= size, "Insufficient Memory to Copy Data");
	ASSERT(pointer != nullptr);
	memcpy((uint8_t*)(pointer) + offsetInByte, (uint8_t*)data, sizeInByte);
}

VulkanVertexBuffer::VulkanVertexBuffer(std::shared_ptr<VulkanAPI> api, BufferUsageHint usage, uint32_t sizeInByte) : m_usage(usage)
{
	VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	VkMemoryPropertyFlags properties = VkTypeConverter::from(usage, bufferUsage);
	m_buffer = std::make_shared<VulkanBuffer>(api, bufferUsage, properties, sizeInByte);
}

void VulkanVertexBuffer::copy(std::shared_ptr<VulkanAPI> api, VkCommandBuffer commandBuffer, void* data, uint32_t offsetInByte, uint32_t sizeInByte)
{
	m_buffer->copy(api, commandBuffer, data, offsetInByte, sizeInByte);
}

void VulkanVertexBuffer::destroy(std::shared_ptr<VulkanAPI> api)
{
	m_buffer->destroy(api);
}


VulkanIndexBuffer::VulkanIndexBuffer(std::shared_ptr<VulkanAPI> api, IndexType indexType, BufferUsageHint usage, uint32_t sizeInByte) : m_usage(usage)
{
	m_indexType = VkTypeConverter::from(indexType);
	VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

	VkMemoryPropertyFlags properties = VkTypeConverter::from(usage, bufferUsage);
	m_buffer = std::make_shared<VulkanBuffer>(api, bufferUsage, properties, sizeInByte);
}

void VulkanIndexBuffer::copy(std::shared_ptr<VulkanAPI> api, VkCommandBuffer commandBuffer, void* data, uint32_t offsetInByte, uint32_t sizeInByte)
{
	m_buffer->copy(api, commandBuffer, data, offsetInByte, sizeInByte);
}

void VulkanIndexBuffer::destroy(std::shared_ptr<VulkanAPI> api)
{
	m_buffer->destroy(api);
}


VulkanUniformBuffer::VulkanUniformBuffer(std::shared_ptr<VulkanAPI> api, BufferUsageHint usage, uint32_t sizeInByte) : m_usage(usage)
{
	VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	VkMemoryPropertyFlags properties = VkTypeConverter::from(usage, bufferUsage);
	m_buffer = std::make_shared<VulkanBuffer>(api, bufferUsage, properties, sizeInByte);
	
	m_bufferInfo.buffer = m_buffer->buffer;
	m_bufferInfo.offset = 0;
	m_bufferInfo.range = VK_WHOLE_SIZE;
}

void VulkanUniformBuffer::copy(std::shared_ptr<VulkanAPI> api, VkCommandBuffer commandBuffer, void* data, uint32_t offsetInByte, uint32_t sizeInByte)
{
	m_buffer->copy(api, commandBuffer, data, offsetInByte, sizeInByte);
}

void VulkanUniformBuffer::destroy(std::shared_ptr<VulkanAPI> api)
{
	m_buffer->destroy(api);
}
