#pragma once

#include <memory>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Crystal
{

class VulkanContext
{
public:
	VkAllocationCallbacks*   m_allocator = nullptr;
	VkInstance               m_instance = VK_NULL_HANDLE;
	VkPhysicalDevice         m_physicalDevice = VK_NULL_HANDLE;
	VkDevice                 m_device = VK_NULL_HANDLE;
	uint32_t                 m_queueFamily = (uint32_t)-1;
	VkQueue                  m_queue = VK_NULL_HANDLE;
	VkDebugReportCallbackEXT m_debugReport = VK_NULL_HANDLE;
	VkPipelineCache          m_pipelineCache = VK_NULL_HANDLE;
	VkDescriptorPool         m_descriptorPool = VK_NULL_HANDLE;

	ImGui_ImplVulkanH_Window m_mainWindowData;
	int                      m_minImageCount = 2;
	bool                     m_swapChainRebuild = false;

	void Initialize(GLFWwindow *window);
	void Shutdown(void);

	void BeginFrame(void);
	void EndFrame(void);

	struct Texture
	{
		VkImage image;
		VkDeviceMemory imageMemory;
		VkImageView imageView;
		VkSampler sampler;
		VkDescriptorSet descriptorSet;

		uint32_t width, height;
	};

	VulkanContext::Texture CreateTexture(const char *file);
	void DestroyTexture(VulkanContext::Texture &texture);

private:
	void FramePresent(ImGui_ImplVulkanH_Window* wd);
	void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data);
	void CleanupVulkanWindow(void);
	void CleanupVulkan(void);
	void SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height);
	void SetupVulkan(ImVector<const char*> instance_extensions);
	VkPhysicalDevice SetupVulkan_SelectPhysicalDevice(void);
	bool IsExtensionAvailable(const ImVector<VkExtensionProperties>& properties, const char* extension);

	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	VkCommandBuffer VulkanContext::BeginSingleTimeCommands(void);
	void VulkanContext::EndSingleTimeCommands(VkCommandBuffer commandBuffer);
	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void VulkanContext::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	GLFWwindow *m_window;
};

}