#pragma once

#include "renderer/renderpass.h"
#include "vulkan_common.h"
#include <vector>

class VulkanAPI;
class VulkanTexture;
class VulkanRenderPass : public RenderPass
{
public:
	// @TODO shortcut function for swapchain 
	// clean it up
	VulkanRenderPass(VkRenderPass renderPass, int width, int height)
	{
		m_width = width;
		m_height = height;
		m_renderPass = renderPass;
	}
	VulkanRenderPass(std::shared_ptr<VulkanAPI> api, const RenderPassDescription& description);


	void set_width(uint32_t width) { m_width = width; }
	void set_height(uint32_t height) { m_height = height; }


	uint32_t get_width() override { return m_width; }
	uint32_t get_height() override { return m_height; };



	VkRenderPass get_renderpass() { return m_renderPass; }

	void destroy(std::shared_ptr<VulkanAPI> api);
private:

	VkRenderPass m_renderPass{};
	uint32_t m_width;
	uint32_t m_height;
	VkRenderPass create_renderpass(VkDevice device, const RenderPassDescription& desc);
};