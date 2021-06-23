#include "vulkan_common.h"


QueueFamilyIndices find_queue_families_indices(VkPhysicalDevice physicalDevice)
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	QueueFamilyIndices familyIndices = {};
	for (uint32_t i = 0; i < queueFamilyCount; ++i)
	{
		VkQueueFlags flag = queueFamilies[i].queueFlags;
		if (flag & VK_QUEUE_GRAPHICS_BIT && flag & VK_QUEUE_COMPUTE_BIT)
		{
			familyIndices.graphicsFamily = i;
			break;
		}
	}
	return familyIndices;
}

bool check_validation_layer_availability(const std::vector<const char*>& validationLayers)
{
	uint32_t layerCount = 0;
	VK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));

	std::vector<VkLayerProperties> availableLayers(layerCount);
	VK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()));

	for (const char* neededLayer : validationLayers)
	{
		for (const auto& availableLayer : availableLayers)
		{
			if (std::strcmp(neededLayer, availableLayer.layerName) == 0)
			{
				return true;
			}
		}
	}

	return false;
}


VkPhysicalDevice select_physical_device(const std::vector<VkPhysicalDevice> physicalDevices)
{
	VkPhysicalDevice selected = physicalDevices[0];

	Debug_Log("--- GPUs ---");
	Debug_Log("Total GPU: %d", physicalDevices.size());
	for (const auto& physicalDevice : physicalDevices)
	{
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		Debug_Log("%s", properties.deviceName);
#if USE_DISCRETE_GPU
		if (properties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			selected = physicalDevice;
#else
		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
			selected = physicalDevice;
#endif
	}
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(selected, &properties);
	Debug_Log("Selected GPU: %s", properties.deviceName);
	return selected;
}


VkSurfaceFormatKHR choose_surface_format(const std::vector<VkSurfaceFormatKHR>& supportedFormats)
{
	for (const auto& surfaceFormat : supportedFormats)
	{
		if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB && surfaceFormat.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
			return surfaceFormat;
	}
	return supportedFormats[0];
}

VkPresentModeKHR choose_present_mode(const std::vector<VkPresentModeKHR>& presentModes)
{
	for (const auto& presentMode : presentModes)
	{
		if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
			return presentMode;
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

uint32_t select_memory_type(const VkPhysicalDeviceMemoryProperties& memoryProperties, uint32_t memoryTypeBits, VkMemoryPropertyFlags flags)
{
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		if ((memoryTypeBits & (1 << i)) != 0 && (memoryProperties.memoryTypes[i].propertyFlags & flags) == flags)
		{
			return i;
		}
	}

	assert("No compatible memory type found");
	return ~0u;
}


VkImageView create_image_view(VkDevice device, VkImage image, VkFormat imageFormat)
{
	VkImageAspectFlags aspect = imageFormat == VK_FORMAT_D32_SFLOAT ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	VkImageViewCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	createInfo.image = image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = imageFormat;
	createInfo.subresourceRange.aspectMask = aspect;
	//createInfo.subresourceRange.baseArrayLayer = 1;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.layerCount = 1;
	VkImageView imageView = 0;
	VK_CHECK(vkCreateImageView(device, &createInfo, nullptr, &imageView));
	return imageView;
}

VkFramebuffer create_framebuffer(VkDevice device, VkRenderPass renderPass, int width, int height, std::vector<VkImageView> imageViews)
{
	VkFramebufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	createInfo.renderPass = renderPass;
	createInfo.attachmentCount = static_cast<uint32_t>(imageViews.size());
	createInfo.pAttachments = imageViews.data();
	createInfo.width = (uint32_t)width;
	createInfo.height = (uint32_t)height;
	createInfo.layers = 1;

	VkFramebuffer framebuffer = 0;
	VK_CHECK(vkCreateFramebuffer(device, &createInfo, nullptr, &framebuffer));
	return framebuffer;
}

void create_image(VulkanImage& result, VkDevice device, const VkPhysicalDeviceMemoryProperties& memoryProperties, uint32_t width, uint32_t height, VkImageUsageFlags usage, VkFormat format)
{
	VkImageCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	createInfo.imageType = VK_IMAGE_TYPE_2D;
	createInfo.format = format;
	createInfo.extent = { width, height, 1 };
	createInfo.mipLevels = 1;
	createInfo.arrayLayers = 1;
	createInfo.samples = VK_SAMPLE_COUNT_1_BIT;

	createInfo.usage = usage;
	createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	createInfo.queueFamilyIndexCount = 0;
	createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkImage image = 0;
	VK_CHECK(vkCreateImage(device, &createInfo, nullptr, &image));

	VkMemoryRequirements memoryRequirement = {};
	vkGetImageMemoryRequirements(device, image, &memoryRequirement);
	uint32_t memoryTypeIndex = select_memory_type(memoryProperties, memoryRequirement.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VkMemoryAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	allocateInfo.allocationSize = memoryRequirement.size;
	allocateInfo.memoryTypeIndex = memoryTypeIndex;

	VkDeviceMemory memory = 0;
	VK_CHECK(vkAllocateMemory(device, &allocateInfo, nullptr, &memory));

	VK_CHECK(vkBindImageMemory(device, image, memory, 0));

	result.image = image;
	result.memory = memory;
	result.imageView = create_image_view(device, image, format);
}

void destroy_image(const VulkanImage& image, VkDevice device)
{
	vkDestroyImageView(device, image.imageView, nullptr);
	vkDestroyImage(device, image.image, 0);
	vkFreeMemory(device, image.memory, 0);
}


VkImageMemoryBarrier image_barrier(VkImage image,
	VkAccessFlags srcAccessMask,
	VkAccessFlags dstAccessMask,
	VkImageLayout oldLayout,
	VkImageLayout newLayout,
	VkImageAspectFlagBits mask = VK_IMAGE_ASPECT_COLOR_BIT)
{
	VkImageMemoryBarrier barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	barrier.srcAccessMask = srcAccessMask;
	barrier.dstAccessMask = dstAccessMask;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = mask;
	barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
	barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
	return barrier;
}


