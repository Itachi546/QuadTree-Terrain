#pragma once

#include "renderer/graphics_api.h"
#include "vulkan_common.h"
#include <vector>

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

	VkInstance get_instance() { return m_Instance; }
	VkPhysicalDevice get_physical_device() { return m_PhysicalDevice; }
	QueueFamilyIndices get_queue_family_indices() { return m_QueueFamilyIndices; }
	VkQueue get_queue() { return m_GraphicsQueue; }

	VkDescriptorPool get_descriptor_pool() { return m_DescriptorPool; }

	void destroy() override;
private:
	VkInstance m_Instance;
	VkDebugUtilsMessengerEXT m_DebugMessenger;
	VkPhysicalDevice m_PhysicalDevice;
	VkDevice m_Device;
	VkQueue m_GraphicsQueue;
	QueueFamilyIndices m_QueueFamilyIndices;
	VkPhysicalDeviceMemoryProperties m_MemoryProps;
	VkPhysicalDeviceProperties m_physicalDeviceProperties;
	VkDescriptorPool m_DescriptorPool;

	VkInstance create_instance(GLFWwindow* window);
	VkDebugUtilsMessengerEXT create_debug_messenger(VkInstance instance);
	VkDevice create_device(VkInstance instance, VkPhysicalDevice physicalDevice, QueueFamilyIndices queueFamilyIndices);

	VkDescriptorPool create_descriptor_pool(VkDevice device);

};
