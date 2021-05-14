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

	VkDescriptorImageInfo* get_image_info()
	{
		return &m_imageInfo;
	}
private:
	VkImage m_image = 0;
	VkImageView m_imageView = 0;
	VkDeviceMemory m_memory = 0;
	VkSampler m_sampler = 0;
	bool m_hasSampler = false;
	VkDescriptorImageInfo m_imageInfo = {};

	void create_image(VkDevice device, VkPhysicalDeviceMemoryProperties memProps, VkImageUsageFlags usage, VkImageType imageType, VkFormat format, uint32_t width, uint32_t height);
	void create_image_view(VkDevice device, VkImageAspectFlags aspectMask, VkFormat format);
	void create_sampler(VkDevice device, SamplerDescription* desc);
};