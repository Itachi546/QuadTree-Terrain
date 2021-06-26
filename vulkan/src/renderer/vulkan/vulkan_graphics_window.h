#pragma once

#include "renderer/graphics_window.h"
#include "input/keyboard.h"
#include <chrono>

#include "vulkan_includes.h"

struct GLFWwindow;
class GraphicsAPI;
class VulkanSwapchain;

struct UserData
{
	std::shared_ptr<Keyboard> keyboard;
	std::shared_ptr<Mouse> mouse;
	int width = 800, height = 600;
	bool resized = false;
};

struct ImGui_ImplVulkan_InitInfo;

class VulkanGraphicsWindow : public GraphicsWindow
{
public:
	VulkanGraphicsWindow(std::shared_ptr<GraphicsAPI>& graphicsAPI, int width, int height, const char* title, bool fullScreen = false);
	void run(double frameRate) override;
	void destroy(std::shared_ptr<GraphicsAPI> m_graphicsAPI);

	virtual std::shared_ptr<Keyboard> get_keyboard() override
	{
		return m_userdata.keyboard;
	}

	virtual std::shared_ptr<Mouse> get_mouse() override
	{
		return m_userdata.mouse;
	}

	void wait_for_event();
	
	int get_width() override { return m_userdata.width; };
	int get_height() override { return m_userdata.height; };
	float get_frame_time() override { return m_frameTimer; };

	GLFWwindow* get_window() { return m_window; }
	
	void init_imgui(ImGui_ImplVulkan_InitInfo* initInfo, VkRenderPass rp, VkCommandBuffer commandBuffer, VkCommandPool commandPool);
	void render_imgui_frame(VkCommandBuffer commandBuffer);
	void destroy_imgui();
private:
	GLFWwindow* m_window = nullptr;
	UserData m_userdata;

	// Last frame time
	float m_frameTimer = 1.0f;
	float m_timer = 0.0f;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_lastTimestamp;
	float m_timerSpeed = 0.25f;
	uint32_t m_frameCounter = 0;
	uint32_t m_lastFPS = 0;

	void imgui_new_frame();


};