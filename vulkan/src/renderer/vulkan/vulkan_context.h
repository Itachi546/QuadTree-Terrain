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

	void acquire_swapchain_image() override;
	void begin() override;

	void begin_compute() override;
	void end_compute() override;

	void begin_renderpass(RenderPass* renderPass, Framebuffer* framebuffer) override;
	void end_renderpass() override;

	void set_pipeline(Pipeline* pipeline) override;

	void copy(VertexBuffer* buffer, void* data, uint32_t offsetInByte, uint32_t sizeInByte) override;
	void copy(IndexBuffer* buffer, void* data, uint32_t offsetInByte, uint32_t sizeInByte) override;
	void copy(UniformBuffer* buffer, void* data, uint32_t offsetInByte, uint32_t sizeInByte) override;
	void copy(ShaderStorageBuffer* buffer, void* data, uint32_t offsetInByte, uint32_t sizeInByte) override;

	void copy(Texture* texture, void* data, uint32_t sizeInByte);


	void set_shader_bindings(ShaderBindings** bindings, uint32_t count) override;
	void set_buffer(VertexBuffer* buffer, uint32_t offset) override;
	void set_buffer(IndexBuffer* buffer, uint32_t offset) override;

	void transition_layout_for_shader_read(Texture** texture, uint32_t count) override;
	void transition_layout_for_compute_read(Texture** texture, uint32_t count) override;

	void set_uniform(ShaderStage shaderStage, uint32_t offset, uint32_t size, void* data) override;
	void set_line_width(float width) override;

	void draw(uint32_t vertexCount) override;
	void draw_indexed(uint32_t indexCount) override;

	void dispatch_compute(uint32_t workGroupSizeX, uint32_t workGroupSizeY, uint32_t workGroupSizeZ) override;
	RenderPass* get_global_renderpass() override
	{
		return reinterpret_cast<RenderPass*>(m_globalRenderPass);
	}

	GraphicsWindow* get_window() override;

	void end() override;
	void present() override;
	void destroy();

private:
	VkCommandPool m_commandPool;
	VkCommandBuffer m_commandBuffer;
	VkCommandBuffer m_tempCommandBuffer;
	VkCommandPool m_tempCommandPool;

	// Compute Shader Acquire and Release Semaphore
	VkSemaphore m_computeAcquireSemaphore;
	VkSemaphore m_computeReleaseSemaphore;

	std::shared_ptr<VulkanSwapchain> m_swapchain;
	VulkanGraphicsWindow* m_window;

	VulkanRenderPass* m_globalRenderPass = nullptr;
	VulkanRenderPass* m_activeRenderPass = nullptr;
	VulkanPipeline* m_activePipeline = nullptr;

	std::shared_ptr<VulkanAPI> m_api;

	VkRenderPass create_global_renderpass(VkDevice device, VkFormat format);
	VkCommandPool create_command_pool(VkDevice device, uint32_t familyIndex);
};