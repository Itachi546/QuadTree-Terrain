#pragma once

#include "renderer/pipeline.h"
#include "vulkan_includes.h"

class VulkanAPI;

struct Shader;
class VulkanPipeline : public Pipeline
{
public:
	VulkanPipeline(std::shared_ptr<VulkanAPI> m_api, const PipelineDescription& desc);
	VkPipeline get_pipeline() { return m_pipeline; }
	VkPipelineLayout get_layout() { return m_layout; }

	VkPipelineBindPoint get_bind_point() { return m_bindPoint; }

	void destroy(std::shared_ptr<VulkanAPI> m_api);
private:
	VkPipelineLayout create_pipeline_layout(VkDevice device, const std::vector<VkDescriptorSetLayout>& setLayouts, const std::vector<VkPushConstantRange> pushConstatRanges);
	VkPipeline create_pipeline(VkDevice device, VkPipelineLayout layout, VkRenderPass renderPass, const PipelineDescription& desc, const std::vector<Shader>& shaders);

	VkPipelineLayout m_layout;
	VkPipeline m_pipeline;
	VkPipelineBindPoint m_bindPoint;
	std::vector<VkDescriptorSetLayout> m_descSetLayouts;
};