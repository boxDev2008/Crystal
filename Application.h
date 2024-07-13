#pragma once

#include "WindowPlugin.h"
#include "VulkanContext.h"

#include <vector>

namespace Crystal
{

class Application
{
public:
	void Run(void);
	void RenderAllWindows(void);
	bool CheckForWindowWithPath(std::filesystem::path &path);
	void AddWindowPlugin(std::shared_ptr<WindowPlugin> window);
	void ManageFreedCache(void);

	void SetLastWindow(std::shared_ptr<WindowPlugin> &window) { m_lastWindow = window; }
	std::shared_ptr<WindowPlugin> &GetLastWindow(void) { return m_lastWindow; }

	ImVec2 GetGlfwWindowPosition(void);
	ImVec2 GetGlfwWindowSize(void);

private:
	std::vector<std::shared_ptr<WindowPlugin>> m_windows;
	std::vector<uint32_t> m_freedWindowIndices;
	std::shared_ptr<WindowPlugin> m_lastWindow;
	VulkanContext m_vkContext;
	GLFWwindow *m_glfwWindow;
};

std::shared_ptr<Application> CreateApplication(void);

}