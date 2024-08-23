#pragma once

#include "Renderer.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

namespace Crystal
{

class VulkanRenderer : public Renderer
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

	VulkanRenderer(void) = default;
	VulkanRenderer(PlatformWindow *window);
	~VulkanRenderer(void);

	void BeginFrame(void) override;
	void EndFrame(void) override;

	Texture *CreateTexture(const char *file, Filter filter) override;
	void DestroyTexture(Texture *texture) override;

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
	VkCommandBuffer VulkanRenderer::BeginSingleTimeCommands(void);
	void VulkanRenderer::EndSingleTimeCommands(VkCommandBuffer commandBuffer);
	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void VulkanRenderer::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
};

}