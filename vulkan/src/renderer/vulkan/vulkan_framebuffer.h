#pragma once

#include "vulkan_includes.h"
#include "renderer/framebuffer.h"
#include <vector>

class VulkanTexture;
class VulkanAPI;
class Texture;
class RenderPass;

class VulkanFramebuffer : public Framebuffer
{
public:

	VulkanFramebuffer(std::shared_ptr<VulkanAPI> api, const FramebufferDescription& desc, RenderPass* rp);

	VkFramebuffer get_framebuffer() { return m_framebuffer; }

	void transition_layout(VkCommandBuffer commandBuffer, VkImageLayout newLayout);

	Texture* get_color_attachment(uint32_t index) override
	{
		ASSERT(index <= static_cast<uint32_t>(m_colorImages.size()));
		return reinterpret_cast<Texture*>(m_colorImages[index]);
	}

	Texture* get_depth_attachment()
	{
		if(m_depthImage)
			return reinterpret_cast<Texture*>(m_depthImage);
		return nullptr;
	}
	bool has_color_attachment() { return m_colorImages.size() > 0; }
	bool has_depth_attachment() { return m_hasDepthAttachment; }

	// Skip image is used in case of swapchain
	void destroy(std::shared_ptr<VulkanAPI> api, bool skipImage = false);

	void resize(uint32_t width, uint32_t height);

	~VulkanFramebuffer() {}

private:
	std::vector<VulkanTexture*> m_colorImages{};
	VulkanTexture* m_depthImage = nullptr;
	VkFramebuffer m_framebuffer{};
	bool m_hasDepthAttachment = false;

};