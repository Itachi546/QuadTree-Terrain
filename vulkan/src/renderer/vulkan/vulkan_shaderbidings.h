#pragma once

#include "renderer/shaderbinding.h"
#include "vulkan_includes.h"
#include "vulkan_buffer.h"
#include "vulkan_texture.h"

class VulkanShaderBindings : public ShaderBindings
{
public:
	void set_buffer(UniformBuffer* ubo, uint32_t binding) override
	{
		VkWriteDescriptorSet descriptor = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		descriptor.dstSet = 0;
		descriptor.dstBinding = binding;
		descriptor.descriptorCount = 1;
		descriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		VulkanUniformBuffer* buffer = reinterpret_cast<VulkanUniformBuffer*>(ubo);

		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = buffer->get_buffer();

		VkDeviceSize offset = 0;
		bufferInfo.offset = 0;
		bufferInfo.range = VK_WHOLE_SIZE;
		descriptor.pBufferInfo = buffer->get_buffer_info();

		descriptorSets.push_back(descriptor);
	}

	virtual void set_buffer(ShaderStorageBuffer* ubo, uint32_t binding)
	{
		VkWriteDescriptorSet descriptor = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		descriptor.dstSet = 0;
		descriptor.dstBinding = binding;
		descriptor.descriptorCount = 1;
		descriptor.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

		VulkanShaderStorageBuffer* buffer = reinterpret_cast<VulkanShaderStorageBuffer*>(ubo);

		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = buffer->get_buffer();

		VkDeviceSize offset = 0;
		bufferInfo.offset = 0;
		bufferInfo.range = VK_WHOLE_SIZE;
		descriptor.pBufferInfo = buffer->get_buffer_info();
		descriptorSets.push_back(descriptor);
	}

	void set_texture_sampler(Texture* texture, uint32_t binding) override
	{
		VkWriteDescriptorSet descriptor = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		descriptor.dstSet = 0;
		descriptor.dstBinding = binding;
		descriptor.descriptorCount = 1;
		descriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		VulkanTexture* vkTexture = reinterpret_cast<VulkanTexture*>(texture);
		descriptor.pImageInfo = vkTexture->get_sampler_descriptor_info();
		descriptorSets.push_back(descriptor);
	}

	void set_storage_image(Texture* texture, uint32_t binding) override
	{
		VkWriteDescriptorSet descriptor = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		descriptor.dstSet = 0;
		descriptor.dstBinding = binding;
		descriptor.descriptorCount = 1;
		descriptor.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

		VulkanTexture* vkTexture = reinterpret_cast<VulkanTexture*>(texture);
		descriptor.pImageInfo = vkTexture->get_storage_image_descriptor_info();
		descriptorSets.push_back(descriptor);
	}

	std::vector<VkWriteDescriptorSet> descriptorSets;
};