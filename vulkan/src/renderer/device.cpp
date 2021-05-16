#include "device.h"
#include "vulkan/vulkan_api.h"
#include "vulkan/vulkan_graphics_window.h"
#include "vulkan/vulkan_pipeline.h"
#include "vulkan/vulkan_renderpass.h"
#include "vulkan/vulkan_context.h"
#include "vulkan/vulkan_buffer.h"
#include "vulkan/vulkan_shaderbidings.h"
#include "vulkan/vulkan_texture.h"
#include "vulkan/vulkan_framebuffer.h"

std::shared_ptr<GraphicsAPI> Device::graphicsAPI = {};
uint64_t Device::totalMemoryAllocated = 0;
GraphicsWindow* Device::create_window(int width, int height, const char* title)
{
	return new VulkanGraphicsWindow(width, height, title, Device::graphicsAPI);
}

Pipeline* Device::create_pipeline(const PipelineDescription& desc)
{
	return new VulkanPipeline(std::static_pointer_cast<VulkanAPI>(Device::graphicsAPI), desc);
}

RenderPass* Device::create_renderpass(const RenderPassDescription& desc)
{
	return new VulkanRenderPass(std::static_pointer_cast<VulkanAPI>(Device::graphicsAPI), desc);
}

Framebuffer* Device::create_framebuffer(const FramebufferDescription& desc, RenderPass* rp)
{
	return new VulkanFramebuffer(std::static_pointer_cast<VulkanAPI>(Device::graphicsAPI), desc, rp);
}

Context* Device::create_context(GraphicsWindow* window)
{
	static Context* context = new VulkanContext(std::static_pointer_cast<VulkanAPI>(Device::graphicsAPI), reinterpret_cast<VulkanGraphicsWindow*>(window));
	return context;
}

VertexBuffer* Device::create_vertexbuffer(BufferUsageHint usage, uint32_t sizeInByte)
{
	totalMemoryAllocated += sizeInByte;
	return new VulkanVertexBuffer(std::static_pointer_cast<VulkanAPI>(Device::graphicsAPI), usage, sizeInByte);
}

IndexBuffer* Device::create_indexbuffer(BufferUsageHint usage, IndexType indexType, uint32_t sizeInByte)
{
	totalMemoryAllocated += sizeInByte;
	return new VulkanIndexBuffer(std::static_pointer_cast<VulkanAPI>(Device::graphicsAPI), indexType, usage, sizeInByte);
}

UniformBuffer* Device::create_uniformbuffer(BufferUsageHint usage, uint32_t sizeInByte)
{
	totalMemoryAllocated += sizeInByte;
	return new VulkanUniformBuffer(std::static_pointer_cast<VulkanAPI>(Device::graphicsAPI), usage, sizeInByte);
}

Texture* Device::create_texture(const TextureDescription& desc)
{
	return new VulkanTexture(std::static_pointer_cast<VulkanAPI>(Device::graphicsAPI), desc);
}

ShaderBindings* Device::create_shader_bindings()
{
	return new VulkanShaderBindings();
}

void Device::destroy_window(GraphicsWindow* window)
{
	VulkanGraphicsWindow* vkWindow = reinterpret_cast<VulkanGraphicsWindow*>(window);
	vkWindow->destroy(graphicsAPI);
	delete window;
}

void Device::destroy_pipeline(Pipeline* pipeline)
{
	VulkanPipeline* vkPipeline = reinterpret_cast<VulkanPipeline*>(pipeline);
	vkPipeline->destroy(std::static_pointer_cast<VulkanAPI>(Device::graphicsAPI));
	delete pipeline;
}

void Device::destroy_renderpass(RenderPass* renderPass)
{
	VulkanRenderPass* vkRenderPass = reinterpret_cast<VulkanRenderPass*>(renderPass);
	vkRenderPass->destroy(std::static_pointer_cast<VulkanAPI>(Device::graphicsAPI));
	delete renderPass;
}

void Device::destroy_framebuffer(Framebuffer* framebuffer)
{
	VulkanFramebuffer* vkFramebuffer = reinterpret_cast<VulkanFramebuffer*>(framebuffer);
	vkFramebuffer->destroy(std::static_pointer_cast<VulkanAPI>(Device::graphicsAPI));
	delete framebuffer;
}

void Device::destroy_context(Context* context)
{
	VulkanContext* vkContext = reinterpret_cast<VulkanContext*>(context);
	vkContext->destroy();
	delete context;
}

void Device::destroy_buffer(VertexBuffer* buffer)
{
	totalMemoryAllocated -= buffer->get_size();
	VulkanVertexBuffer* vkBuffer = reinterpret_cast<VulkanVertexBuffer*>(buffer);
	vkBuffer->destroy(std::static_pointer_cast<VulkanAPI>(Device::graphicsAPI));
	ASSERT(totalMemoryAllocated >= 0);
}

void Device::destroy_buffer(IndexBuffer* buffer)
{
	totalMemoryAllocated -= buffer->get_size();
	VulkanIndexBuffer* vkBuffer = reinterpret_cast<VulkanIndexBuffer*>(buffer);
	vkBuffer->destroy(std::static_pointer_cast<VulkanAPI>(Device::graphicsAPI));
	ASSERT(totalMemoryAllocated >= 0);
}

void Device::destroy_buffer(UniformBuffer* buffer)
{
	totalMemoryAllocated -= buffer->get_size();
	VulkanUniformBuffer* vkBuffer = reinterpret_cast<VulkanUniformBuffer*>(buffer);
	vkBuffer->destroy(std::static_pointer_cast<VulkanAPI>(Device::graphicsAPI));
	ASSERT(totalMemoryAllocated >= 0);
}

void Device::destroy_texture(Texture* texture)
{
	VulkanTexture* vkTexture = reinterpret_cast<VulkanTexture*>(texture);
	vkTexture->destroy(std::static_pointer_cast<VulkanAPI>(Device::graphicsAPI));
	delete texture;
}

void Device::destroy_shader_bindings(ShaderBindings* bindings)
{
	delete bindings;
}
