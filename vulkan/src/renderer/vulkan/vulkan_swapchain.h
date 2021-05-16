#pragma once

#include "renderer/swapchain.h"
#include "vulkan_common.h"
#include "vulkan_type_converter.h"
#include <vector>

struct GLFWwindow;
class VulkanAPI;

class VulkanSwapchain : public Swapchain
{
public:
	friend class VulkanContext;

	VulkanSwapchain(std::shared_ptr<VulkanAPI> api, GLFWwindow* window, uint32_t width, uint32_t height);
	void create_required_attachments(std::shared_ptr<VulkanAPI> api, VkRenderPass renderPass);
	VkResult acquire_next_image(VkDevice device);
	VkResult present(VkCommandBuffer commandBuffer, VkQueue queue);
	void destroy(std::shared_ptr<VulkanAPI> api);
	void resize_swapchain(std::shared_ptr<VulkanAPI> api, VkRenderPass renderPass, uint32_t width, uint32_t height);

	// can be called only between beginCommandBuffer and endCommandBuffer
	void transition_depth_image_layout(VkCommandBuffer commandBuffer);
	VkFormat get_format()
	{
		return m_surfaceFormat.format;
	}
private:

	std::shared_ptr<VulkanAPI> m_api;

	VkSurfaceKHR m_surface;
	VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;

	std::vector<VkImage> m_images;
	std::vector<VkImageView> m_imageViews;
	std::vector<VkFramebuffer> m_framebuffers;
	std::vector<VulkanImage> m_depthImage;

	VkSurfaceFormatKHR m_surfaceFormat;
	VkExtent2D m_extent;
	uint32_t m_currentImageIndex = 0;

	const uint32_t MAX_FRAME_IN_FLIGHT = 2;
	uint32_t m_currentFrame = 0;

	std::vector<VkSemaphore> m_acquireSemaphore;
	std::vector<VkSemaphore> m_releaseSemaphore;
	std::vector<VkFence> m_inFlightFences;
	std::vector<VkFence> m_imagesInFlight;

	VkSurfaceKHR create_surface(VkInstance instance, GLFWwindow* window);
	void create_swapchain(std::shared_ptr<VulkanAPI> api, uint32_t width, uint32_t height);
};