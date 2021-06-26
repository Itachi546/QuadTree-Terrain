#include "vulkan_query.h"
#include "vulkan_api.h"

VulkanQuery::VulkanQuery(std::shared_ptr<VulkanAPI> m_api, uint32_t queryCount) : m_queryCount(queryCount)
{
	VkQueryPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO };
    createInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
    createInfo.queryCount = queryCount;

	VK_CHECK(vkCreateQueryPool(m_api->get_device(), &createInfo, 0, &m_pool));
}

void VulkanQuery::destroy(std::shared_ptr<VulkanAPI> m_api)
{
    vkDestroyQueryPool(m_api->get_device(), m_pool, 0);
}
