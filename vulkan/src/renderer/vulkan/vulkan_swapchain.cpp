#include "vulkan_swapchain.h"
#include "vulkan_api.h"
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

VkSurfaceKHR VulkanSwapchain::create_surface(VkInstance instance, GLFWwindow* window)
{
	VkWin32SurfaceCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
	createInfo.hinstance = GetModuleHandle(nullptr);
	createInfo.hwnd = glfwGetWin32Window(window);
	VkSurfaceKHR surface = 0;
	VK_CHECK(vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface));
	return surface;
}

static VkExtent2D choose_swapchain_extend(const VkSurfaceCapabilitiesKHR& surfaceCapabilities, uint32_t width, uint32_t height)
{
	VkExtent2D actualExtent = { width, height };
	actualExtent.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, actualExtent.width));
	actualExtent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, actualExtent.height));
	return actualExtent;
}

static VkSemaphore create_semaphore(VkDevice device)
{
	//Create Semaphore
	VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	//semaphoreCreateInfo.flags = VK_SEMAPHORE_TYPE_BINARY;

	VkSemaphore semaphore = 0;
	VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphore));
	return semaphore;
}

static VkFence create_fence(VkDevice device)
{
	VkFenceCreateInfo createInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkFence fence = 0;
	VK_CHECK(vkCreateFence(device, &createInfo, nullptr, &fence));
	return fence;
}

struct SwapchainCapabilities
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	std::vector<VkSurfaceFormatKHR> supportedFormats;
	std::vector<VkPresentModeKHR> presentModes;
};


static SwapchainCapabilities get_swapchain_capabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const QueueFamilyIndices& familyIndices)
{
	SwapchainCapabilities caps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &caps.surfaceCapabilities);

	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
	caps.supportedFormats.resize(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, caps.supportedFormats.data());

	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
	caps.presentModes.resize(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, caps.presentModes.data());
	return caps;
}

void VulkanSwapchain::create_swapchain(std::shared_ptr<VulkanAPI> api, uint32_t width, uint32_t height)
{
	SwapchainCapabilities caps = get_swapchain_capabilities(api->m_PhysicalDevice, m_surface, api->m_QueueFamilyIndices);
	m_surfaceFormat = choose_surface_format(caps.supportedFormats);
	m_extent = choose_swapchain_extend(caps.surfaceCapabilities, width, height);

	VkSwapchainCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	createInfo.surface = m_surface;
	createInfo.minImageCount = std::min(caps.surfaceCapabilities.minImageCount + 1, caps.surfaceCapabilities.maxImageCount);
	createInfo.imageFormat = m_surfaceFormat.format;
	createInfo.imageColorSpace = m_surfaceFormat.colorSpace;
	createInfo.imageExtent = m_extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

	uint32_t familyIndices[] = { api->m_QueueFamilyIndices.graphicsFamily };
	createInfo.queueFamilyIndexCount = 1;
	createInfo.pQueueFamilyIndices = familyIndices;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	createInfo.preTransform = caps.surfaceCapabilities.currentTransform;
	createInfo.presentMode = choose_present_mode(caps.presentModes);
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = m_swapchain;

	VkDevice device = api->m_Device;
	VK_CHECK(vkCreateSwapchainKHR(device, &createInfo, 0, &m_swapchain));
}

void VulkanSwapchain::resize_swapchain(std::shared_ptr<VulkanAPI> api, VkRenderPass renderPass, uint32_t width, uint32_t height)
{
	VkDevice device = api->m_Device;
	vkDeviceWaitIdle(device);

	for (auto& framebuffer : m_framebuffers)
		vkDestroyFramebuffer(device, framebuffer, nullptr);

	for (auto& depthImage : m_depthImage)
		destroy_image(depthImage, device);

	for (auto& imageView : m_imageViews)
		vkDestroyImageView(device, imageView, 0);

	m_framebuffers.clear();
	m_depthImage.clear();
	m_imageViews.clear();
	m_images.clear();

	//vkDestroySwapchainKHR(device, m_swapchain, 0);

	create_swapchain(api, width, height);
	create_required_attachments(api, renderPass);
}


VulkanSwapchain::VulkanSwapchain(std::shared_ptr<VulkanAPI> api, GLFWwindow* window, uint32_t width, uint32_t height)
{
	m_api = api;

	m_surface = create_surface(api->m_Instance, window);
	VkBool32 presentSupport = false;
	VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(api->m_PhysicalDevice, api->m_QueueFamilyIndices.graphicsFamily, m_surface, &presentSupport));
	assert(presentSupport != false);
	create_swapchain(api, width, height);

	m_acquireSemaphore.resize(MAX_FRAME_IN_FLIGHT);
	m_releaseSemaphore.resize(MAX_FRAME_IN_FLIGHT);
	m_inFlightFences.resize(MAX_FRAME_IN_FLIGHT);

	VkDevice device = api->m_Device;
	for (uint32_t i = 0; i < MAX_FRAME_IN_FLIGHT; ++i)
	{
		m_acquireSemaphore[i] = create_semaphore(device);
		m_releaseSemaphore[i] = create_semaphore(device);
		m_inFlightFences[i] = create_fence(device);
	}
}


void VulkanSwapchain::create_required_attachments(std::shared_ptr<VulkanAPI> api, VkRenderPass renderPass)
{
	VkDevice device = api->m_Device;
	uint32_t imageCount = 0;
	VK_CHECK(vkGetSwapchainImagesKHR(device, m_swapchain, &imageCount, nullptr));
	m_images.resize(imageCount);
	VK_CHECK(vkGetSwapchainImagesKHR(device, m_swapchain, &imageCount, m_images.data()));

	m_imageViews.resize(imageCount);
	m_framebuffers.resize(imageCount);
	m_depthImage.resize(imageCount);
	for (uint32_t i = 0; i < imageCount; ++i)
	{
		m_imageViews[i] = create_image_view(device, m_images[i], m_surfaceFormat.format);
		create_image(m_depthImage[i], device, api->m_MemoryProps, m_extent.width, m_extent.height, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_FORMAT_D32_SFLOAT);
		std::vector<VkImageView> imageViews;
		imageViews.push_back(m_imageViews[i]);
		imageViews.push_back(m_depthImage[i].imageView);
		m_framebuffers[i] = create_framebuffer(device, renderPass, m_extent.width, m_extent.height, imageViews);
	}

	if (m_imagesInFlight.size() == 0)
		m_imagesInFlight.resize(imageCount, VK_NULL_HANDLE);
}

void VulkanSwapchain::transition_depth_image_layout(VkCommandBuffer commandBuffer)
{
	VkImageMemoryBarrier barriers[] = {
		image_barrier(m_depthImage[m_currentImageIndex].image, 0, 0,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		VK_IMAGE_ASPECT_DEPTH_BIT)
	};

	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		VK_DEPENDENCY_BY_REGION_BIT,
		0, VK_NULL_HANDLE,
		0, VK_NULL_HANDLE,
		ARRAYSIZE(barriers), barriers);

}

VkResult VulkanSwapchain::acquire_next_image(VkDevice device)
{
	vkWaitForFences(device, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);
	VkResult result = vkAcquireNextImageKHR(device, m_swapchain, ~0ull, m_acquireSemaphore[m_currentFrame], 0, &m_currentImageIndex);
	if (m_imagesInFlight[m_currentImageIndex] != VK_NULL_HANDLE)
		vkWaitForFences(device, 1, &m_imagesInFlight[m_currentImageIndex], VK_TRUE, UINT64_MAX);

	m_imagesInFlight[m_currentImageIndex] = m_inFlightFences[m_currentFrame];

	return result;
}

VkResult VulkanSwapchain::present(VkCommandBuffer commandBuffer, VkQueue queue)
{

	//	After returning the swapchain image index, the image is not ready
	//  we specify semaphore that will be signalled when image is ready

	vkResetFences(m_api->m_Device, 1, &m_inFlightFences[m_currentFrame]);

	VkPipelineStageFlags submitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &m_acquireSemaphore[m_currentFrame];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &m_releaseSemaphore[m_currentFrame];

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.pWaitDstStageMask = &submitStageMask;

	VK_CHECK(vkQueueSubmit(queue, 1, &submitInfo, m_inFlightFences[m_currentFrame]));

	VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_swapchain;
	presentInfo.pImageIndices = &m_currentImageIndex;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_releaseSemaphore[m_currentFrame];

	m_currentFrame = (m_currentFrame + 1) % MAX_FRAME_IN_FLIGHT;
	VkResult result = vkQueuePresentKHR(queue, &presentInfo);
	return result;
}


void VulkanSwapchain::destroy(std::shared_ptr<VulkanAPI> api)
{
	VkDevice device = api->m_Device;
	for (auto& framebuffer : m_framebuffers)
		vkDestroyFramebuffer(device, framebuffer, nullptr);

	for (auto& depthImage : m_depthImage)
		destroy_image(depthImage, device);

	for (auto& imageView : m_imageViews)
		vkDestroyImageView(device, imageView, 0);

	for (std::size_t i = 0; i < m_inFlightFences.size(); ++i)
		vkDestroyFence(device, m_inFlightFences[i], 0);
	for (std::size_t i = 0; i < m_acquireSemaphore.size(); ++i)
	{
		vkDestroySemaphore(device, m_acquireSemaphore[i], 0);
		vkDestroySemaphore(device, m_releaseSemaphore[i], 0);
	}

	vkDestroySwapchainKHR(device, m_swapchain, 0);
	vkDestroySurfaceKHR(api->m_Instance, m_surface, 0);
}