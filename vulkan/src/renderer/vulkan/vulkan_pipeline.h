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
	VkDescriptorSet get_descriptor_set() { return m_descriptorSet; }
	void destroy(std::shared_ptr<VulkanAPI> m_api);
private:
	VkPipelineLayout create_pipeline_layout(VkDevice device, const std::vector<VkDescriptorSetLayout>& setLayouts, const std::vector<VkPushConstantRange> pushConstatRanges);
	VkPipeline create_graphics_pipeline(VkDevice device, VkPipelineLayout layout, VkRenderPass renderPass, const PipelineDescription& desc, const std::vector<Shader>& shaders);
	VkPipeline create_compute_pipeline(VkDevice device, VkPipelineLayout layout, Shader& shader);

	VkPipelineLayout m_layout;
	VkPipeline m_pipeline;
	VkPipelineBindPoint m_bindPoint;
	std::vector<VkDescriptorSetLayout> m_descSetLayouts;
	VkDescriptorSet m_descriptorSet;
};
