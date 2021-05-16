#include "vulkan_renderpass.h"
#include "vulkan_api.h"
#include "vulkan_type_converter.h"
#include "vulkan_texture.h"
#include <algorithm>


VkRenderPass VulkanRenderPass::create_renderpass(VkDevice device, const RenderPassDescription& desc)
{
	uint32_t attachmentCount = desc.attachmentCount;
	ASSERT(attachmentCount > 0);

	std::vector<VkAttachmentDescription> attachments(attachmentCount);

	std::vector<VkAttachmentReference> colorAttachmentRefs;
	VkAttachmentReference depthAttachmentRef = {};
	bool hasDepthAttachment = false;
	for (uint32_t i = 0; i < attachmentCount; ++i)
	{
		const Attachment& attachment = desc.attachments[i];
		attachments[i].format = VkTypeConverter::from(attachment.format);
		attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		if (attachment.attachmentType == TextureType::DepthStencil)
		{
			attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depthAttachmentRef = { desc.attachments[i].index, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
			hasDepthAttachment = true;
		}
		else
		{
			attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[i].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			VkAttachmentReference colorAttachment = { desc.attachments[i].index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
			colorAttachmentRefs.push_back(colorAttachment);
		}
	}

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
	subpass.pColorAttachments = colorAttachmentRefs.data();
	if(hasDepthAttachment)
		subpass.pDepthStencilAttachment = &depthAttachmentRef;


	VkRenderPassCreateInfo createInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subpass;
	createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	createInfo.pAttachments = attachments.data();

	VkRenderPass renderPass = 0;
	VK_CHECK(vkCreateRenderPass(device, &createInfo, nullptr, &renderPass));
	return renderPass;
}

VulkanRenderPass::VulkanRenderPass(std::shared_ptr<VulkanAPI> api, const RenderPassDescription& description)
{
	m_width = description.width;
	m_height = description.height;

	VkDevice device = api->m_Device;
	m_renderPass = create_renderpass(device, description);
}


void VulkanRenderPass::destroy(std::shared_ptr<VulkanAPI> api)
{
	VkDevice device = api->m_Device;
	vkDestroyRenderPass(device, m_renderPass, 0);
}
