#pragma once

#include "WindowPlugin.h"
#include "VulkanContext.h"
#include "BuildErrorHandler.h"

#include <vector>

namespace Crystal
{

class Application
{
public:
	void Run(void);
	void RenderAllWindows(void);
	void ManageFreedCache(void);

	ImVec2 GetGlfwWindowPosition(void);
	ImVec2 GetGlfwWindowSize(void);
	ImVec2 GetMouseCursorPosition(void);

	std::shared_ptr<WindowPlugin> CheckForWindowWithPath(std::filesystem::path &path);
	std::vector<std::shared_ptr<WindowPlugin>> &GetAllWindowPlugins(void) { return m_windows; }
	void AddWindowPlugin(std::shared_ptr<WindowPlugin> window);

	VulkanContext &GetVulkanContext(void) { return m_vkContext; }
	BuildErrorHandler &GetBuildErrorHandler(void) { return m_buildErrorHandler; }

	void SetMainDirectoryPath(const std::filesystem::path &path) { m_mainDirectory = path;  }
	std::filesystem::path GetMainDirectoryPath(void) const { return m_mainDirectory; }

	void SetLastWindow(WindowPlugin *window) { m_lastWindow = window; }
	WindowPlugin *GetLastWindow(void) const { return m_lastWindow; }

private:
	std::vector<std::shared_ptr<WindowPlugin>> m_windows;
	std::vector<uint32_t> m_freedWindowIndices;

	WindowPlugin *m_lastWindow;

	std::filesystem::path m_mainDirectory;

	BuildErrorHandler m_buildErrorHandler;
	VulkanContext m_vkContext;
	GLFWwindow *m_glfwWindow;
};

std::shared_ptr<Application> CreateApplication(void);

}