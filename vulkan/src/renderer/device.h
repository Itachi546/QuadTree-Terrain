#pragma once

#include "graphics_enums.h"
#include <memory>


class GraphicsWindow;
class VertexBuffer;
class IndexBuffer;
class UniformBuffer;
class ShaderStorageBuffer;
class IndirectBuffer;

class GraphicsAPI;
class Pipeline;
class RenderPass;
class Framebuffer;
class Context;
class ShaderBindings;
class Texture;

struct TextureDescription;
struct RenderPassDescription;
struct PipelineDescription;
struct FramebufferDescription;

class GpuTimestampQuery;
/*
  Renderpass(Attachments, vkCmdBeginRenderPass),
  Framebuffer(renderPass, Attachments)
  Pipeline(pipelineLayout, renderPass, vertexShader, fragmentShader, vkCmdBindPipeline)
*/

class Device
{
public:
	static GraphicsWindow* create_window(int width, int height, const char* title, bool fullScreen = false);
	static Pipeline* create_pipeline(const PipelineDescription& desc);
	static RenderPass* create_renderpass(const RenderPassDescription& desc);
	static Framebuffer* create_framebuffer(const FramebufferDescription& desc, RenderPass* rp);

	static Context* create_context(GraphicsWindow* window);

	static VertexBuffer* create_vertexbuffer(BufferUsageHint usage, uint32_t sizeInByte);
	static IndexBuffer* create_indexbuffer(BufferUsageHint usage, IndexType indexType, uint32_t sizeInByte);
	static UniformBuffer* create_uniformbuffer(BufferUsageHint usage, uint32_t sizeInByte);
	static ShaderStorageBuffer* create_shader_storage_buffer(BufferUsageHint usage, uint32_t sizeInByte);
	static IndirectBuffer* create_indirect_buffer(BufferUsageHint usage, uint32_t sizeInByte);

	static Texture* create_texture(const TextureDescription& desc);
	static GpuTimestampQuery* create_query(uint32_t queryCount);


	static ShaderBindings* create_shader_bindings();

	static void destroy_window(GraphicsWindow* window);
	static void destroy_pipeline(Pipeline* pipeline);
	static void destroy_renderpass(RenderPass* renderPass);
	static void destroy_framebuffer(Framebuffer* framebuffer);
	static void destroy_context(Context* context);

	static void destroy_buffer(VertexBuffer* buffer);
	static void destroy_buffer(IndexBuffer* buffer);
	static void destroy_buffer(UniformBuffer* buffer);
	static void destroy_buffer(ShaderStorageBuffer* buffer);
	static void destroy_buffer(IndirectBuffer* buffer);

	static void destroy_texture(Texture* texture);
	static void destroy_shader_bindings(ShaderBindings* bindings);
	static void destroy_query(GpuTimestampQuery* query);

	static uint64_t get_total_memory_allocated() { return totalMemoryAllocated; }
private:
	static std::shared_ptr<GraphicsAPI> graphicsAPI;
	static uint64_t totalMemoryAllocated;
};
