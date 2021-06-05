#include "vulkan_context.h"
#include "vulkan_api.h"
#include "vulkan_pipeline.h"
#include "vulkan_renderpass.h"
#include "vulkan_graphics_window.h"
#include "vulkan_swapchain.h"
#include "vulkan_buffer.h"
#include "vulkan_shaderbidings.h"
#include "vulkan_framebuffer.h"

#include "imgui/imgui_impl_vulkan.h"

static void check_vk_result(VkResult err)
{
	if (err == 0)
		return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}

VkCommandPool VulkanContext::create_command_pool(VkDevice device, uint32_t familyIndex)
{
	VkCommandPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	createInfo.queueFamilyIndex = familyIndex;
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VkCommandPool commandPool = 0;
	VK_CHECK(vkCreateCommandPool(device, &createInfo, nullptr, &commandPool));
	return commandPool;
}

VkRenderPass VulkanContext::create_global_renderpass(VkDevice device, VkFormat format)
{
	VkAttachmentDescription attachments[2] = {};
	attachments[0].format = format;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	attachments[1].format = VK_FORMAT_D32_SFLOAT;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkRenderPassCreateInfo createInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	createInfo.attachmentCount = 2;
	createInfo.pAttachments = attachments;

	VkAttachmentReference colorAttachment = {};
	colorAttachment.attachment = 0;
	colorAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachment = {};
	depthAttachment.attachment = 1;
	depthAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDesc = {};
	subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDesc.colorAttachmentCount = 1;
	subpassDesc.pColorAttachments = &colorAttachment;
	subpassDesc.pDepthStencilAttachment = &depthAttachment;

	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subpassDesc;

	VkSubpassDependency subpassDependency = {};
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.dstSubpass = 0;
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	subpassDependency.srcAccessMask = 0;
	subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	createInfo.dependencyCount = 1;
	createInfo.pDependencies = &subpassDependency;

	VkRenderPass renderPass = 0;
	VK_CHECK(vkCreateRenderPass(device, &createInfo, 0, &renderPass));
	return renderPass;
}

VulkanContext::VulkanContext(std::shared_ptr<VulkanAPI> api, VulkanGraphicsWindow* window)
{

	m_api = api;
	VkDevice device = api->m_Device;
	m_window = window;

	uint32_t width = window->get_width();
	uint32_t height = window->get_height();

	m_swapchain = std::make_shared<VulkanSwapchain>(api, window->get_window(), width, height);

	VkRenderPass renderPass = create_global_renderpass(device, m_swapchain->get_format());
	m_globalRenderPass = new VulkanRenderPass(renderPass, width, height);

	// Swapchain
	m_swapchain->create_required_attachments(api, renderPass);

	uint32_t graphicsFamilyIndex = api->m_QueueFamilyIndices.graphicsFamily;
	m_commandPool = create_command_pool(device, graphicsFamilyIndex);
	VkCommandBufferAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocateInfo.commandBufferCount = 1;
	allocateInfo.commandPool = m_commandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	VK_CHECK(vkAllocateCommandBuffers(api->m_Device, &allocateInfo, &m_commandBuffer));


	m_tempCommandPool = create_command_pool(device, graphicsFamilyIndex);
	allocateInfo.commandBufferCount = 1;
	allocateInfo.commandPool = m_tempCommandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	VK_CHECK(vkAllocateCommandBuffers(api->m_Device, &allocateInfo, &m_tempCommandBuffer));

	{
		ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.Instance = m_api->get_instance();
		initInfo.PhysicalDevice = m_api->get_physical_device();
		initInfo.Device = device;
		initInfo.QueueFamily = m_api->get_queue_family_indices().graphicsFamily;
		initInfo.Queue = m_api->get_queue();
		initInfo.PipelineCache = 0;
		initInfo.DescriptorPool = m_api->get_descriptor_pool();
		initInfo.Allocator = 0;
		initInfo.MinImageCount = m_swapchain->get_min_image_count();
		initInfo.ImageCount = m_swapchain->get_image_count();
		initInfo.CheckVkResultFn = check_vk_result;
		m_window->init_imgui(&initInfo, m_globalRenderPass->get_renderpass(), m_tempCommandBuffer, m_tempCommandPool);
	}

	VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &m_computeReleaseSemaphore));
	VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &m_computeAcquireSemaphore));
}

void VulkanContext::acquire_swapchain_image()
{
	VkResult result = m_swapchain->acquire_next_image(m_api->m_Device);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		uint32_t width = m_window->get_width();
		uint32_t height = m_window->get_height();
		m_window->wait_for_event();
		m_swapchain->resize_swapchain(m_api, m_globalRenderPass->get_renderpass(), width, height);
		m_globalRenderPass->set_width(m_swapchain->m_extent.width);
		m_globalRenderPass->set_height(m_swapchain->m_extent.height);
	}
}

void VulkanContext::begin()
{
	VK_CHECK(vkResetCommandPool(m_api->m_Device, m_commandPool, 0));
	VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	VK_CHECK(vkBeginCommandBuffer(m_commandBuffer, &beginInfo));

}

void VulkanContext::begin_compute()
{
	VK_CHECK(vkResetCommandPool(m_api->m_Device, m_commandPool, 0));
	VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	VK_CHECK(vkBeginCommandBuffer(m_commandBuffer, &beginInfo));
}

void VulkanContext::end_compute()
{
	VK_CHECK(vkEndCommandBuffer(m_commandBuffer));
	VkPipelineStageFlags submitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandBuffer;
	submitInfo.pWaitDstStageMask = &submitStageMask;
	VK_CHECK(vkQueueSubmit(m_api->get_queue(), 1, &submitInfo, 0));
	vkDeviceWaitIdle(m_api->get_device());
}

void VulkanContext::begin_renderpass(RenderPass* rp, Framebuffer* framebuffer)
{
	if (rp == nullptr)
		m_activeRenderPass = m_globalRenderPass;
	else
		m_activeRenderPass = reinterpret_cast<VulkanRenderPass*>(rp);

	bool defaultRenderPass = m_activeRenderPass == m_globalRenderPass;

	VkRenderPassBeginInfo beginPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
	beginPassInfo.renderPass = m_activeRenderPass->get_renderpass();
	uint32_t width = m_activeRenderPass->get_width();
	uint32_t height = m_activeRenderPass->get_height();
	beginPassInfo.renderArea.extent.width = width;
	beginPassInfo.renderArea.extent.height = height;
	VulkanFramebuffer* vkFramebuffer = nullptr;

	std::vector<VkClearValue> clearValues;
	VkClearValue depthStencil = {};
	depthStencil.depthStencil = { m_clearValues.depth, 0u };

	if (framebuffer != nullptr)
	{
		vkFramebuffer = reinterpret_cast<VulkanFramebuffer*>(framebuffer);
		beginPassInfo.framebuffer = vkFramebuffer->get_framebuffer();
		if(vkFramebuffer->has_color_attachment())
			clearValues.push_back({ m_clearValues.r, m_clearValues.g, m_clearValues.b, m_clearValues.a });
		if (vkFramebuffer->has_depth_attachment())
			clearValues.push_back(depthStencil);
		vkFramebuffer->transition_layout(m_commandBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	}
	else
	{
		clearValues.push_back({ m_clearValues.r, m_clearValues.g, m_clearValues.b, m_clearValues.a });
		beginPassInfo.framebuffer = m_swapchain->m_framebuffers[m_swapchain->m_currentImageIndex];
		clearValues.push_back(depthStencil);
		m_swapchain->transition_depth_image_layout(m_commandBuffer);
	}

	beginPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	beginPassInfo.pClearValues = clearValues.data();
	
	vkCmdBeginRenderPass(m_commandBuffer, &beginPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	VkViewport viewport = { 0.0f, float(height), float(width), -float(height), 0.0f, 1.0f };
	vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);
	VkRect2D scissorRect = { {0, 0}, {(uint32_t)width, (uint32_t)height} };
	vkCmdSetScissor(m_commandBuffer, 0, 1, &scissorRect);
}

void VulkanContext::end_renderpass()
{
	if (m_activeRenderPass == m_globalRenderPass)
		m_window->render_imgui_frame(m_commandBuffer);

	vkCmdEndRenderPass(m_commandBuffer);
}

void VulkanContext::set_pipeline(Pipeline* pipeline)
{
	ASSERT(pipeline != nullptr);
	m_activePipeline = reinterpret_cast<VulkanPipeline*>(pipeline);
	vkCmdBindPipeline(m_commandBuffer, m_activePipeline->get_bind_point(), m_activePipeline->get_pipeline());
}


void VulkanContext::copy(VertexBuffer* buffer, void* data, uint32_t offsetInByte, uint32_t sizeInByte)
{
	VulkanVertexBuffer* vkBuffer = reinterpret_cast<VulkanVertexBuffer*>(buffer);
	vkBuffer->copy(m_api, m_tempCommandBuffer, data, offsetInByte, sizeInByte);
}

void VulkanContext::copy(IndexBuffer* buffer, void* data, uint32_t offsetInByte, uint32_t sizeInByte)
{
	VulkanIndexBuffer* vkBuffer = reinterpret_cast<VulkanIndexBuffer*>(buffer);
	vkBuffer->copy(m_api, m_tempCommandBuffer, data, offsetInByte, sizeInByte);
}

void VulkanContext::copy(UniformBuffer* buffer, void* data, uint32_t offsetInByte, uint32_t sizeInByte)
{
	VulkanUniformBuffer* vkBuffer = reinterpret_cast<VulkanUniformBuffer*>(buffer);
	vkBuffer->copy(m_api, m_tempCommandBuffer, data, offsetInByte, sizeInByte);
}

void VulkanContext::copy(ShaderStorageBuffer* buffer, void* data, uint32_t offsetInByte, uint32_t sizeInByte)
{
	VulkanShaderStorageBuffer* vkBuffer = reinterpret_cast<VulkanShaderStorageBuffer*>(buffer);
	vkBuffer->copy(m_api, m_tempCommandBuffer, data, offsetInByte, sizeInByte);
}

void VulkanContext::copy(Texture* texture, void* data, uint32_t sizeInByte)
{
	VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VkCommandBuffer commandBuffer = m_tempCommandBuffer;
	vkResetCommandBuffer(commandBuffer, 0);
	VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

	VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	VulkanBuffer stagingBuffer(m_api, bufferUsage, properties, sizeInByte);
	stagingBuffer.copy(data, 0, sizeInByte);

	VulkanTexture* vkTexture = reinterpret_cast<VulkanTexture*>(texture);
	
	VkImageMemoryBarrier barrier = image_barrier(vkTexture->get_image(), 0, 0, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vkTexture->get_image_aspect());
	vkCmdPipelineBarrier(m_tempCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, 0, 0, 0, 1, &barrier);
	vkTexture->set_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = vkTexture->get_image_aspect();
	region.imageSubresource.layerCount = 1;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.mipLevel = 0;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { vkTexture->get_width(), vkTexture->get_height(), 1};
	vkCmdCopyBufferToImage(commandBuffer, stagingBuffer.buffer, vkTexture->get_image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	VK_CHECK(vkEndCommandBuffer(commandBuffer));

	VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	VkQueue queue = m_api->m_GraphicsQueue;

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkDeviceWaitIdle(m_api->m_Device);
	stagingBuffer.destroy(m_api);
}

void VulkanContext::set_shader_bindings(ShaderBindings** shaderBindings, uint32_t count)
{
	if (shaderBindings == nullptr)
		return;

	std::vector<VkWriteDescriptorSet> writeSetsTotal;
	for (uint32_t i = 0; i < count; ++i)
	{
		VulkanShaderBindings* bindings = reinterpret_cast<VulkanShaderBindings*>(shaderBindings[i]);
		auto& writeSets = bindings->descriptorSets;
		writeSetsTotal.insert(writeSetsTotal.end(), writeSets.begin(), writeSets.end());
	}

	VkDescriptorSet descriptorSet = m_activePipeline->get_descriptor_set();
	for (auto& writeSets : writeSetsTotal)
		writeSets.dstSet = descriptorSet;

	vkUpdateDescriptorSets(m_api->get_device(), static_cast<uint32_t>(writeSetsTotal.size()), writeSetsTotal.data(), 0, nullptr);
	vkCmdBindDescriptorSets(m_commandBuffer, m_activePipeline->get_bind_point(), m_activePipeline->get_layout(), 0, 1, &descriptorSet, 0, 0);
}

void VulkanContext::set_buffer(VertexBuffer* buffer, uint32_t offsetInByte)
{
	VulkanVertexBuffer* vkBuffer = reinterpret_cast<VulkanVertexBuffer*>(buffer);
	VkBuffer buffers[] = {vkBuffer->get_buffer()};

	VkDeviceSize offset = offsetInByte;
	vkCmdBindVertexBuffers(m_commandBuffer, 0, ARRAYSIZE(buffers), buffers, &offset);
}

void VulkanContext::set_buffer(IndexBuffer* buffer, uint32_t offsetInByte)
{
	VulkanIndexBuffer* vkBuffer = reinterpret_cast<VulkanIndexBuffer*>(buffer);
	VkDeviceSize offset = offsetInByte;
	vkCmdBindIndexBuffer(m_commandBuffer, vkBuffer->get_buffer(), offset, vkBuffer->get_index_type());
}

void VulkanContext::transition_layout_for_shader_read(Texture** texture, uint32_t count)
{
	std::vector<VkImageMemoryBarrier> barriers(count);
	for (uint32_t i = 0; i < count; ++i)
	{
		VulkanTexture* vkTexture = reinterpret_cast<VulkanTexture*>(texture[i]);
		// @TODO For depth image initial layout is always undefined
		// maybe we need to keep track of it in image

		VkImageAspectFlagBits aspect = vkTexture->get_image_aspect();
		if (vkTexture->get_layout() == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			return;

		barriers[i] = image_barrier(vkTexture->get_image(), 0, 0,
			vkTexture->get_layout(),
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, aspect);
		vkTexture->set_layout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}
	vkCmdPipelineBarrier(m_commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		0, 0, nullptr,
		0, nullptr,
		count, barriers.data());

}

void VulkanContext::transition_layout_for_compute_read(Texture** texture, uint32_t count)
{
	std::vector<VkImageMemoryBarrier> barriers(count);
	for (uint32_t i = 0; i < count; ++i)
	{
		VulkanTexture* vkTexture = reinterpret_cast<VulkanTexture*>(texture[i]);
		// @TODO For depth image initial layout is always undefined
		// maybe we need to keep track of it in image

		VkImageAspectFlagBits aspect = vkTexture->get_image_aspect();
		if (vkTexture->get_layout() == VK_IMAGE_LAYOUT_GENERAL)
			return;

		barriers[i] = image_barrier(vkTexture->get_image(), 0, 0,
			vkTexture->get_layout(),
			VK_IMAGE_LAYOUT_GENERAL, aspect);

		vkTexture->set_layout(VK_IMAGE_LAYOUT_GENERAL);
	}

	vkCmdPipelineBarrier(m_commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		0, 0, nullptr,
		0, nullptr,
		count, barriers.data());

}

void VulkanContext::set_uniform(ShaderStage shaderStage, uint32_t offset, uint32_t size, void* data)
{
	vkCmdPushConstants(m_commandBuffer, m_activePipeline->get_layout(), VkTypeConverter::from(shaderStage), offset, size, data);
}

void VulkanContext::set_line_width(float width)
{
	vkCmdSetLineWidth(m_commandBuffer, width);
}

void VulkanContext::draw(uint32_t vertexCount)
{
	vkCmdDraw(m_commandBuffer, vertexCount, 1, 0, 0);
}

void VulkanContext::draw_indexed(uint32_t indexCount)
{
	vkCmdDrawIndexed(m_commandBuffer, indexCount, 1, 0, 0, 0);
}

void VulkanContext::dispatch_compute(uint32_t workGroupSizeX, uint32_t workGroupSizeY, uint32_t workGroupSizeZ)
{
	vkCmdDispatch(m_commandBuffer, workGroupSizeX, workGroupSizeY, workGroupSizeZ);
}

GraphicsWindow* VulkanContext::get_window()
{
	return reinterpret_cast<GraphicsWindow*>(m_window);
}

void VulkanContext::end()
{
	vkEndCommandBuffer(m_commandBuffer);
}

void VulkanContext::present()
{
	VkQueue queue = m_api->m_GraphicsQueue;
	VkResult result = m_swapchain->present(m_commandBuffer, queue);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		uint32_t width = m_window->get_width();
		uint32_t height = m_window->get_height();

		m_window->wait_for_event();
		m_swapchain->resize_swapchain(m_api, m_globalRenderPass->get_renderpass(), width, height);
		m_globalRenderPass->set_width(width);
		m_globalRenderPass->set_height(height);
	}
	vkDeviceWaitIdle(m_api->m_Device);
}

void VulkanContext::destroy()
{
	m_window->destroy_imgui();
	VkDevice device = m_api->get_device();
	vkDestroySemaphore(device, m_computeAcquireSemaphore, 0);
	vkDestroySemaphore(device, m_computeReleaseSemaphore, 0);

	vkFreeCommandBuffers(device, m_commandPool, 1, &m_commandBuffer);
	vkDestroyCommandPool(device, m_commandPool, 0);

	vkFreeCommandBuffers(device, m_tempCommandPool, 1, &m_tempCommandBuffer);
	vkDestroyCommandPool(device, m_tempCommandPool, 0);

	m_globalRenderPass->destroy(m_api);
	m_swapchain->destroy(m_api);
}
