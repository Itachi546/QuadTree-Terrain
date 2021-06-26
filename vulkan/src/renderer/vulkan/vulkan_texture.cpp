#include "vulkan_texture.h"
#include "vulkan_api.h"
#include "vulkan_type_converter.h"

void VulkanTexture::create_image(VkDevice device, VkPhysicalDeviceMemoryProperties memProps, VkImageUsageFlags usage, VkImageType imageType, VkFormat format, uint32_t width, uint32_t height, uint32_t layerCount)
{
	VkImageCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	if (layerCount == 6)
		createInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

	createInfo.imageType = imageType;
	createInfo.format = format;
	createInfo.extent = { width, height, 1 };
	createInfo.mipLevels = 1;
	createInfo.arrayLayers = layerCount;
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

void VulkanTexture::create_image_view(VkDevice device, VkImageAspectFlags aspectMask, VkFormat format, VkImageViewType imageViewType, uint32_t layerCount)
{
	VkImageViewCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	createInfo.image = m_image;
	createInfo.viewType = imageViewType;
	createInfo.format = format;
	createInfo.subresourceRange.aspectMask = aspectMask;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.layerCount = layerCount;
	VK_CHECK(vkCreateImageView(device, &createInfo, nullptr, &m_imageView));
}

VulkanTexture::VulkanTexture(std::shared_ptr<VulkanAPI> api, const TextureDescription& desc) : m_width(desc.width), m_height(desc.height)
{
	VkDevice device = api->get_device();
	VkPhysicalDeviceMemoryProperties memoryProps = api->get_memory_properties();

	VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	m_aspect = VK_IMAGE_ASPECT_COLOR_BIT;
	if (desc.type == TextureType::DepthStencil)
	{
		m_aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}

	if (desc.flags & TextureFlag::Sampler)
	{
		usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
		ASSERT_MSG(desc.sampler != nullptr, "Failed to set sampler description");
	}
	if (desc.flags & TextureFlag::TransferDst)
		usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	if (desc.flags & TextureFlag::TransferSrc)
		usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	if (desc.flags & TextureFlag::StorageImage)
		usage |= VK_IMAGE_USAGE_STORAGE_BIT;

	ASSERT(desc.type != TextureType::Color3D);
	VkImageType imageType = VK_IMAGE_TYPE_2D;
	VkFormat format = VkTypeConverter::from(desc.format);

	VkImageViewType imageViewType = VK_IMAGE_VIEW_TYPE_2D;
	uint32_t layerCount = 1;
	if (desc.type == TextureType::Cubemap)
	{
		imageViewType = VK_IMAGE_VIEW_TYPE_CUBE;
		layerCount = 6;
	}

	create_image(device, memoryProps, usage, imageType, format, desc.width, desc.height, layerCount);
	create_image_view(device, m_aspect, format, imageViewType, layerCount);

	m_layout = VK_IMAGE_LAYOUT_UNDEFINED;

	if (desc.sampler)
	{
		create_sampler(device, desc.sampler);
		m_hasSampler = true;

		m_samplerInfo.sampler = m_sampler;
		m_samplerInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_samplerInfo.imageView = m_imageView;

		m_storageImageInfo.sampler = m_sampler;
		m_storageImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		m_storageImageInfo.imageView = m_imageView;
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
	createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
	createInfo.unnormalizedCoordinates = VK_FALSE;

	vkCreateSampler(device, &createInfo, 0, &m_sampler);
}

