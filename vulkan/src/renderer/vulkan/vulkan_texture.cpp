#include "vulkan_texture.h"
#include "vulkan_api.h"
#include "vulkan_type_converter.h"

void VulkanTexture::create_image(VkDevice device, VkPhysicalDeviceMemoryProperties memProps, VkImageUsageFlags usage, VkImageType imageType, VkFormat format, uint32_t width, uint32_t height)
{
	VkImageCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	createInfo.imageType = imageType;
	createInfo.format = format;
	createInfo.extent = { width, height, 1 };
	createInfo.mipLevels = 1;
	createInfo.arrayLayers = 1;
	createInfo.samples = VK_SAMPLE_COUNT_1_BIT;

	createInfo.usage = usage;
	createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	createInfo.queueFamilyIndexCount = 0;
	createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VK_CHECK(vkCreateImage(device, &createInfo, nullptr, &m_image));

	VkMemoryRequirements memoryRequirement = {};
	vkGetImageMemoryRequirements(device, m_image, &memoryRequirement);
	uint32_t memoryTypeIndex = select_memory_type(memProps, memoryRequirement.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VkMemoryAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	allocateInfo.allocationSize = memoryRequirement.size;
	allocateInfo.memoryTypeIndex = memoryTypeIndex;

	VK_CHECK(vkAllocateMemory(device, &allocateInfo, nullptr, &m_memory));
	VK_CHECK(vkBindImageMemory(device, m_image, m_memory, 0));
}

void VulkanTexture::create_image_view(VkDevice device, VkImageAspectFlags aspectMask, VkFormat format)
{
	VkImageViewCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	createInfo.image = m_image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = format;
	createInfo.subresourceRange.aspectMask = aspectMask;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.layerCount = 1;
	VK_CHECK(vkCreateImageView(device, &createInfo, nullptr, &m_imageView));
}

VulkanTexture::VulkanTexture(std::shared_ptr<VulkanAPI> api, const TextureDescription& desc)
{
	VkDevice device = api->get_device();
	VkPhysicalDeviceMemoryProperties memoryProps = api->get_memory_properties();

	VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
	if (desc.type == TextureType::DepthStencil)
	{
		aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}

	if (desc.sampler != nullptr)
		usage |= VK_IMAGE_USAGE_SAMPLED_BIT;

	VkImageType imageType = VK_IMAGE_TYPE_2D;
	VkFormat format = VkTypeConverter::from(desc.format);
	create_image(device, memoryProps, usage, imageType, format, desc.width, desc.height);
	create_image_view(device, aspect, format);

	if (desc.sampler != nullptr)
	{
		create_sampler(device, desc.sampler);
		m_hasSampler = true;

		m_imageInfo.sampler = m_sampler;
		m_imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_imageInfo.imageView = m_imageView;

	}
}

void VulkanTexture::destroy(std::shared_ptr<VulkanAPI> api)
{
	VkDevice device = api->get_device();
	if (m_hasSampler)
		vkDestroySampler(device, m_sampler, 0);
	vkDestroyImageView(device, m_imageView, 0);
	vkDestroyImage(device, m_image, 0);
	vkFreeMemory(device, m_memory, 0);
}

void VulkanTexture::create_sampler(VkDevice device, SamplerDescription* desc)
{
	VkSamplerCreateInfo createInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
	createInfo.magFilter = VkTypeConverter::from(desc->magFilter);
	createInfo.minFilter = VkTypeConverter::from(desc->minFilter);

	createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	createInfo.addressModeU = VkTypeConverter::from(desc->wrapU);
	createInfo.addressModeV = VkTypeConverter::from(desc->wrapV);
	createInfo.addressModeW = VkTypeConverter::from(desc->wrapW);
	createInfo.mipLodBias = 0.0f;

	// @TODO enable anisotropy later
	createInfo.anisotropyEnable = false;
	createInfo.maxAnisotropy = 0.0f;
	createInfo.compareEnable = VK_FALSE;
	createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	createInfo.minLod = 0.0f;
	createInfo.maxLod = 0.0f;
	createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
	createInfo.unnormalizedCoordinates = VK_FALSE;

	vkCreateSampler(device, &createInfo, 0, &m_sampler);
}

