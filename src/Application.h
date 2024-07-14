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

	ImVec2 GetGlfwWindowPosition(void);
	ImVec2 GetGlfwWindowSize(void);

	VulkanContext &GetVulkanContext(void) { return m_vkContext; }

private:
	std::vector<std::shared_ptr<WindowPlugin>> m_windows;
	std::vector<uint32_t> m_freedWindowIndices;

	VulkanContext m_vkContext;
	GLFWwindow *m_glfwWindow;
};

std::shared_ptr<Application> CreateApplication(void);

}