#pragma once

#include "vulkan_includes.h"

#ifdef _DEBUG
#define ENABLE_VALIDATION_LAYERS 1
#endif

#define USE_DISCRETE_GPU 1

/*
 *  This causes crash in release build
 *  #define VK_CHECK(expr) assert(expr == VK_SUCCESS)
 */

#define VK_CHECK(expr) \
if(expr == VK_SUCCESS){\
}\
else{\
__debugbreak();\
}\

static constexpr uint32_t MAX_UINT32 = ~0u;

struct QueueFamilyIndices
{
	uint32_t graphicsFamily = MAX_UINT32;
	bool is_complete()
	{
		return (graphicsFamily != MAX_UINT32);
	}
};

struct VulkanImage
{
	VkImage image;
	VkDeviceMemory memory;
	VkImageView imageView;
};


QueueFamilyIndices find_queue_families_indices(VkPhysicalDevice physicalDevice);
bool check_validation_layer_availability(const std::vector<const char*>& validationLayers);
VkPhysicalDevice select_physical_device(const std::vector<VkPhysicalDevice> physicalDevices);
VkSurfaceFormatKHR choose_surface_format(const std::vector<VkSurfaceFormatKHR>& supportedFormats);
VkPresentModeKHR choose_present_mode(const std::vector<VkPresentModeKHR>& presentModes);
uint32_t select_memory_type(const VkPhysicalDeviceMemoryProperties& memoryProperties, uint32_t memoryTypeBits, VkMemoryPropertyFlags flags);

// @TODO redundant
VkImageView create_image_view(VkDevice device, VkImage image, VkFormat imageFormat);
void create_image(VulkanImage& result, VkDevice device, const VkPhysicalDeviceMemoryProperties& memoryProperties, uint32_t width, uint32_t height, VkImageUsageFlags usage, VkFormat format);
void destroy_image(const VulkanImage& image, VkDevice device);
VkFramebuffer create_framebuffer(VkDevice device, VkRenderPass renderPass, int width, int height, std::vector<VkImageView> imageViews);
VkImageMemoryBarrier image_barrier(VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlagBits mask);


