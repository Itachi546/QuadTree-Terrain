#include "vulkan_graphics_window.h"
#include "vulkan_api.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"
#include <GLFW/glfw3.h>


static void resize_callback(GLFWwindow* window, int width, int height)
{
	UserData* userData = reinterpret_cast<UserData*>(glfwGetWindowUserPointer(window));
	ASSERT(userData != nullptr);
	userData->width = width;
	userData->height = height;
	userData->resized = true;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	UserData* userData = reinterpret_cast<UserData*>(glfwGetWindowUserPointer(window));
	ASSERT(userData != nullptr);
	auto keyboard = userData->keyboard;

	if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
		glfwSetWindowShouldClose(window, true);

	if (action == GLFW_RELEASE)
		keyboard->on_key_up(KeyCode(key));
	else
		keyboard->on_key_down(KeyCode(key));
}

static void mouse_callback(GLFWwindow* window, int button, int action, int mods)
{
	UserData* userData = reinterpret_cast<UserData*>(glfwGetWindowUserPointer(window));
	ASSERT(userData != nullptr);
	auto mouse = userData->mouse;

	if (action == GLFW_RELEASE)
		mouse->on_key_up(Button(button));
	else
		mouse->on_key_down(Button(button));

}

VulkanGraphicsWindow::VulkanGraphicsWindow(int width, int height, const char* title, std::shared_ptr<GraphicsAPI>& graphicsAPI)
{
	if (!glfwInit())
		glfwInit();

	m_userdata.width = width;
	m_userdata.height = height;
	m_userdata.keyboard = std::make_shared<Keyboard>();
	m_userdata.mouse = std::make_shared<Mouse>();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_window = glfwCreateWindow(width, height, title, 0, 0);
	glfwSetWindowUserPointer(m_window, &m_userdata);
	glfwSetWindowSizeCallback(m_window, resize_callback);
	glfwSetKeyCallback(m_window, key_callback);
	glfwSetMouseButtonCallback(m_window, mouse_callback);
	ASSERT_MSG(m_window != nullptr, "Failed to create Vulkan Window!");
	
	//glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	std::shared_ptr<VulkanAPI> api = std::make_shared<VulkanAPI>(m_window);
	graphicsAPI = api;
}

void VulkanGraphicsWindow::run(double frameRate)
{
	m_lastTimestamp = std::chrono::high_resolution_clock::now();
	while (!glfwWindowShouldClose(m_window))
	{
		glfwPollEvents();

		imgui_new_frame();

		double x = 0.0, y = 0.0;
		glfwGetCursorPos(m_window, &x, &y);
		m_userdata.mouse->set_mouse_position(float(x), float(y));

		auto start = std::chrono::high_resolution_clock::now();
		on_update_frame();
		on_pre_render_frame();
		on_render_frame();
		on_post_render_frame();

		m_frameCounter++;
		auto end = std::chrono::high_resolution_clock::now();
		auto diff = std::chrono::duration<double, std::milli>(end - start).count();
		m_frameTimer = (float)diff / 1000.0f;

		m_timer += m_timerSpeed * m_frameTimer;
		if (m_timer > 1.0f)
			m_timer -= 1.0f;

		float fpsTimer = (float)std::chrono::duration<double, std::milli>(end - m_lastTimestamp).count();
		if (fpsTimer > 1000.0f)
		{
			m_lastFPS = static_cast<uint32_t>((float)m_frameCounter * (1000.0f / fpsTimer));
			m_frameCounter = 0;
			m_lastTimestamp = end;
		}

		if (m_userdata.resized && _on_resize)
		{
			_on_resize(m_userdata.width, m_userdata.height);
			m_userdata.resized = false;
		}
	}
}

void VulkanGraphicsWindow::destroy(std::shared_ptr<GraphicsAPI> api)
{
	api->destroy();
	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void VulkanGraphicsWindow::wait_for_event()
{
	while(m_userdata.width == 0 || m_userdata.height == 0)
		glfwWaitEvents();
}

void VulkanGraphicsWindow::init_imgui(ImGui_ImplVulkan_InitInfo* initInfo, VkRenderPass rp, VkCommandBuffer commandBuffer, VkCommandPool commandPool)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForVulkan(m_window, true);
	ImGui_ImplVulkan_Init(initInfo, rp);

	VkDevice device = initInfo->Device;
	{
		VkCommandPool command_pool = commandPool;
		VkCommandBuffer command_buffer = commandBuffer;

		VK_CHECK(vkResetCommandPool(device, command_pool, 0));

		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		VK_CHECK(vkBeginCommandBuffer(command_buffer, &begin_info));

		ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

		VkSubmitInfo end_info = {};
		end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		end_info.commandBufferCount = 1;
		end_info.pCommandBuffers = &command_buffer;
		VK_CHECK(vkEndCommandBuffer(command_buffer));
		VK_CHECK(vkQueueSubmit(initInfo->Queue, 1, &end_info, VK_NULL_HANDLE));

		VK_CHECK(vkDeviceWaitIdle(device));
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}
}

void VulkanGraphicsWindow::destroy_imgui()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void VulkanGraphicsWindow::imgui_new_frame()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void VulkanGraphicsWindow::render_imgui_frame(VkCommandBuffer commandBuffer)
{
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}
