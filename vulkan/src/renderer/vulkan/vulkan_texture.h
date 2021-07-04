#pragma once

#include "renderer/texture.h"
#include "vulkan_includes.h"
#include <memory>

struct VulkanImage;
class VulkanAPI;
class VulkanTexture : public Texture
{
public:
	VulkanTexture(std::shared_ptr<VulkanAPI> api, const TextureDescription& desc);
	void destroy(std::shared_ptr<VulkanAPI> api);

	VkImage get_image() { return m_image; }
	VkImageView get_image_view() { return m_imageView; }
	VkSampler get_sampler() { return m_sampler; }

	VkImageAspectFlagBits get_image_aspect() { return m_aspect; }
	VkDescriptorImageInfo* get_sampler_descriptor_info() { return &m_samplerInfo; }

	VkDescriptorImageInfo* get_storage_image_descriptor_info() { return &m_storageImageInfo; }
	VkImageLayout get_layout() { return m_layout; }

	void set_layout(VkImageLayout layout) { m_layout = layout; }

	uint32_t get_height() override { return m_height; }
	uint32_t get_width() override { return m_width; }

private:
	VkImage m_image = 0;
	VkImageView m_imageView = 0;
	VkDeviceMemory m_memory = 0;
	VkSampler m_sampler = 0;
	bool m_hasSampler = false;

	VkDescriptorImageInfo m_samplerInfo = {};
	VkDescriptorImageInfo m_storageImageInfo = {};

	VkImageAspectFlagBits m_aspect;
	VkImageLayout m_layout;

	uint32_t m_width;
	uint32_t m_height;

	void create_image(VkDevice device, VkPhysicalDeviceMemoryProperties memProps, VkImageUsageFlags usage, VkImageType imageType, VkFormat format, uint32_t width, uint32_t height, uint32_t layerCount);
	void create_image_view(VkDevice device, VkImageAspectFlags aspectMask, VkFormat format, VkImageViewType imageViewType, uint32_t layerCount);
	void create_sampler(VkDevice device, SamplerDescription* desc);
};