#pragma once

#include "vulkan_includes.h"
#include "renderer/graphics_enums.h"
#include "renderer/gpu_query.h"

class VulkanAPI;
class VulkanQuery : public GpuTimestampQuery
{
public:
	VulkanQuery(std::shared_ptr<VulkanAPI> m_api, uint32_t queryCount);
	void destroy(std::shared_ptr<VulkanAPI> m_api);

	inline uint32_t get_query_count() { return m_queryCount; }
	inline VkQueryPool get_query_pool() { return m_pool; }
private:
	VkQueryPool m_pool = 0;
	uint32_t m_queryCount;
};