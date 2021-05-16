#include "vulkan_framebuffer.h"
#include "vulkan_texture.h"
#include "vulkan_common.h"
#include "vulkan_api.h"
#include "vulkan_type_converter.h"
#include "vulkan_renderpass.h"

VulkanFramebuffer::VulkanFramebuffer(std::shared_ptr<VulkanAPI> api, const FramebufferDescription& desc, RenderPass* rp)
{
	int attachmentCount = desc.attachmentCount;

	VkPhysicalDeviceMemoryProperties memoryProps = api->get_memory_properties();
	int width = desc.width;
	int height = desc.height;
	for (int i = 0; i < attachmentCount; ++i)
	{
		const TextureDescription& attachment = desc.attachments[i];
		VkFormat format = VkTypeConverter::from(attachment.format);

		if (attachment.type == TextureType::DepthStencil)
		{
			m_depthImage = new VulkanTexture(api, attachment);
			m_hasDepthAttachment = true;
		}
		else
			m_colorImages.push_back(new VulkanTexture(api, attachment));
	}

	std::vector<VkImageView> imageViews;
	for (std::size_t i = 0; i < m_colorImages.size(); ++i)
		imageViews.push_back(m_colorImages[i]->get_image_view());
	if (m_depthImage)
		imageViews.push_back(m_depthImage->get_image_view());

	VkDevice device = api->get_device();
	m_framebuffer = create_framebuffer(device, (reinterpret_cast<VulkanRenderPass*>(rp))->get_renderpass(), width, height, imageViews);

}

void VulkanFramebuffer::transition_layout(VkCommandBuffer commandBuffer, VkImageLayout newLayout)
{
	std::vector<VkImageMemoryBarrier> barriers;
	for (int i = 0; i < m_colorImages.size(); ++i)
	{
		barriers.push_back(image_barrier(m_colorImages[i]->get_image(), 0, 0, VK_IMAGE_LAYOUT_UNDEFINED,
			newLayout, VK_IMAGE_ASPECT_COLOR_BIT));
	}

	if (m_depthImage)
		barriers.push_back(image_barrier(m_depthImage->get_image(), 0, 0, VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT));

	if(barriers.size() > 0)
		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		VK_DEPENDENCY_BY_REGION_BIT, 0, 0, 0, 0,
		static_cast<uint32_t>(barriers.size()), barriers.data());
}

void VulkanFramebuffer::destroy(std::shared_ptr<VulkanAPI> api, bool skipImage)
{
	if (!skipImage)
	{
		for (auto& image : m_colorImages)
		{
			image->destroy(api);
			delete image;
		}
	}

	if (m_hasDepthAttachment)
	{
		m_depthImage->destroy(api);
		delete m_depthImage;
	}
	vkDestroyFramebuffer(api->get_device(), m_framebuffer, 0);
}

void VulkanFramebuffer::resize(uint32_t width, uint32_t height)
{
	ASSERT_MSG(0, "Not implemented");
}
