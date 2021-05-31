#pragma once

#include "renderer/context.h"
#include "vulkan_includes.h"

class VulkanAPI;
class VulkanGraphicsWindow;
class VulkanRenderPass;
class VulkanPipeline;
class VulkanSwapchain;
class Framebuffer;

class VulkanContext : public Context
{

public:
	VulkanContext(std::shared_ptr<VulkanAPI> api, VulkanGraphicsWindow* window);
	void begin() override;

	void begin_renderpass(RenderPass* renderPass, Framebuffer* framebuffer) override;
	void end_renderpass() override;

	void set_graphics_pipeline(Pipeline* pipeline) override;
	
	void copy(VertexBuffer* buffer, void* data, uint32_t offsetInByte, uint32_t sizeInByte) override;
	void copy(IndexBuffer* buffer, void* data, uint32_t offsetInByte, uint32_t sizeInByte) override;
	void copy(UniformBuffer* buffer, void* data, uint32_t offsetInByte, uint32_t sizeInByte) override;
	void set_shader_bindings(ShaderBindings** bindings, uint32_t count) override;
	void set_buffer(VertexBuffer* buffer, uint32_t offset) override;
	void set_buffer(IndexBuffer* buffer, uint32_t offset) override;

	void transition_layout_for_shader_read(Texture* texture, bool depthTexture) override;


	void set_uniform(ShaderStage shaderStage, uint32_t offset, uint32_t size, void* data) override;
	void set_line_width(float width) override;

	void draw(uint32_t vertexCount) override;
	void draw_indexed(uint32_t indexCount) override;

	RenderPass* get_global_renderpass() override
	{
		return reinterpret_cast<RenderPass*>(m_globalRenderPass);
	}

	GraphicsWindow* get_window() override;

	void end() override;
	void destroy();

private:
	VkCommandPool m_commandPool;
	VkCommandBuffer m_commandBuffer;
	VkCommandBuffer m_tempCommandBuffer;
	VkCommandPool m_tempCommandPool;

	std::shared_ptr<VulkanSwapchain> m_swapchain;
	VulkanGraphicsWindow* m_window;

	VulkanRenderPass* m_globalRenderPass = nullptr;
	VulkanRenderPass* m_activeRenderPass = nullptr;
	VulkanPipeline* m_activePipeline = nullptr;

	std::shared_ptr<VulkanAPI> m_api;

	VkRenderPass create_global_renderpass(VkDevice device, VkFormat format);
	VkCommandPool create_command_pool(VkDevice device, uint32_t familyIndex);
};