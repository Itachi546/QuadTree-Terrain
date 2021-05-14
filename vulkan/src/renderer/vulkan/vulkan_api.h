#pragma once

#include "renderer/graphics_api.h"
#include "vulkan_common.h"
#include <vector>

extern PFN_vkCmdPushDescriptorSetKHR vulkanCmdPushDescriptorSetKHR;

struct SwapchainInfo
{
	VkSwapchainKHR swapchain;
	VkExtent2D extend;

	VkSurfaceFormatKHR surfaceFormat;

	std::vector<VkImage> colorImages;
	std::vector<VkImageView> colorImageViews;
	std::vector<VkFramebuffer> framebuffers;
};


struct GLFWwindow;

class VulkanAPI : public GraphicsAPI
{
	friend class VulkanSwapchain;
	friend class VulkanPipeline;
	friend class VulkanRenderPass;
	friend class VulkanContext;
	friend class VulkanBuffer;

public:
	VulkanAPI(GLFWwindow* window);

	VkDevice get_device() { return m_Device; }
	VkPhysicalDeviceMemoryProperties get_memory_properties() { return m_MemoryProps; }
	void destroy() override;
private:
	VkInstance m_Instance;
	VkDebugUtilsMessengerEXT m_DebugMessenger;
	VkPhysicalDevice m_PhysicalDevice;
	VkDevice m_Device;
	VkQueue m_GraphicsQueue;
	QueueFamilyIndices m_QueueFamilyIndices;
	VkPhysicalDeviceMemoryProperties m_MemoryProps;

	VkInstance create_instance(GLFWwindow* window);
	VkDebugUtilsMessengerEXT create_debug_messenger(VkInstance instance);
	VkDevice create_device(VkInstance instance, VkPhysicalDevice physicalDevice, QueueFamilyIndices queueFamilyIndices);

};
