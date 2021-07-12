#include "vulkan_api.h"
#include "vulkan_common.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <algorithm>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{

	const char* type = (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) ? "---- ERROR ----\n " :
		(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) ? "---- WARNING ----\n" : "---- INFO ----\n ";

	if (messageSeverity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
	{
		std::cout << type << pCallbackData->pMessage << std::endl;
		OutputDebugStringA(type);
		OutputDebugStringA(pCallbackData->pMessage);
		__debugbreak();
	}
	return VK_FALSE;
}

VkInstance VulkanAPI::create_instance(GLFWwindow* window)
{
	VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "VulkanRenderer";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_2;


	VkInstanceCreateInfo createInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
#if ENABLE_VALIDATION_LAYERS
	std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	if (check_validation_layer_availability(validationLayers))
	{
		createInfo.ppEnabledLayerNames = validationLayers.data();
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());

		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		Debug_Log("Validation Layer enabled");
	}
#else
	createInfo.enabledLayerCount = 0;
#endif
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkInstance instance = 0;
	VK_CHECK(vkCreateInstance(&createInfo, VK_NULL_HANDLE, &instance));
	return instance;
}

VkDebugUtilsMessengerEXT VulkanAPI::create_debug_messenger(VkInstance instance)
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;

	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

	createInfo.pfnUserCallback = debugCallback;
	VkDebugUtilsMessengerEXT debugMessenger = 0;
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	assert(func != nullptr);

	VK_CHECK(func(instance, &createInfo, nullptr, &debugMessenger));
	return debugMessenger;
}

VkDevice VulkanAPI::create_device(VkInstance instance, VkPhysicalDevice physicalDevice, QueueFamilyIndices queueFamilyIndices)
{
	VkDeviceQueueCreateInfo queueCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
	float queuePriority = 1.0f;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	const char* extensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
	};

	uint32_t propertyCount = 0;
	VK_CHECK(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &propertyCount, nullptr));
	std::vector<VkExtensionProperties> properties(propertyCount);
	VK_CHECK(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &propertyCount, properties.data()));

	bool allExtensionAvailable = true;
	for (int i = 0; i < ARRAYSIZE(extensions); ++i)
	{
		bool available = false;
		for (const auto& property : properties)
		{
			if (std::strcmp(property.extensionName, extensions[i]) == 0)
			{
				available = true;
				break;
			}
		}
		if (available == false)
			Debug_Error("Extension not found: ", std::string(extensions[i]));
		allExtensionAvailable &= available;
	}
	assert(allExtensionAvailable != false);

	VkDeviceCreateInfo createInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	createInfo.queueCreateInfoCount = 1;
	createInfo.pQueueCreateInfos = &queueCreateInfo;
	createInfo.enabledExtensionCount = ARRAYSIZE(extensions);;
	createInfo.ppEnabledExtensionNames = extensions;

	VkPhysicalDeviceFeatures features = {};
	features.fillModeNonSolid = true;
	features.shaderClipDistance = true;
	features.wideLines = true;
	features.multiDrawIndirect = true;
	features.geometryShader = true;
	features.tessellationShader = true;
	features.samplerAnisotropy = true;
	createInfo.pEnabledFeatures = &features;


	VkDevice device = 0;
	VK_CHECK(vkCreateDevice(physicalDevice, &createInfo, 0, &device));
	return device;
}

VkDescriptorPool VulkanAPI::create_descriptor_pool(VkDevice device)
{
	VkDescriptorPoolSize poolSizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};
	VkDescriptorPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	createInfo.maxSets = 1000 * ARRAYSIZE(poolSizes);
	createInfo.poolSizeCount = ARRAYSIZE(poolSizes);
	createInfo.pPoolSizes = poolSizes;

	VkDescriptorPool descriptorPool = 0;
	VK_CHECK(vkCreateDescriptorPool(device, &createInfo, 0, &descriptorPool));
	return descriptorPool;
}

VulkanAPI::VulkanAPI(GLFWwindow* window)
{
	m_Instance = create_instance(window);
#if ENABLE_VALIDATION_LAYERS
	m_DebugMessenger = create_debug_messenger(m_Instance);
#endif

	uint32_t deviceCount = 0;
	VK_CHECK(vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr));
	assert(deviceCount != 0);
	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	VK_CHECK(vkEnumeratePhysicalDevices(m_Instance, &deviceCount, physicalDevices.data()));

	m_PhysicalDevice = select_physical_device(physicalDevices);

	m_QueueFamilyIndices = find_queue_families_indices(m_PhysicalDevice);
	m_Device = create_device(m_Instance, m_PhysicalDevice, m_QueueFamilyIndices);
	m_DescriptorPool = create_descriptor_pool(m_Device);

	vkGetDeviceQueue(m_Device, m_QueueFamilyIndices.graphicsFamily, 0, &m_GraphicsQueue);

	vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &m_MemoryProps);
	vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_physicalDeviceProperties);
}

void VulkanAPI::destroy()
{
	vkDestroyDescriptorPool(m_Device, m_DescriptorPool, 0);
	vkDestroyDevice(m_Device, 0);
#if ENABLE_VALIDATION_LAYERS
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT");
	assert(func != nullptr);
	func(m_Instance, m_DebugMessenger, 0);
#endif
	vkDestroyInstance(m_Instance, 0);
}
