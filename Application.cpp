#include "Application.h"
#include "WindowPlugin.h"
#include "WizardWindow.h"
#include "EditorWindow.h"
#include "ExplorerWindow.h"
#include "Resources.h"

#include "imgui/imgui.h"

#include <stb_image.h>

#include <algorithm>

namespace Crystal
{

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

std::shared_ptr<Application> CreateApplication(void)
{
	return std::make_shared<Application>();
}

void Application::Run(void)
{
    //glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_glfwWindow = glfwCreateWindow(1280, 720, "Crystal", nullptr, nullptr);
    if (!glfwVulkanSupported())
    {
        printf("GLFW: Vulkan Not Supported\n");
        return;
    }

	{
		GLFWimage icon; 
		icon.pixels = stbi_load("res/logo-small.png", &icon.width, &icon.height, 0, 4);
		glfwSetWindowIcon(m_glfwWindow, 1, &icon); 
		stbi_image_free(icon.pixels);
	}

	glfwSetWindowSizeLimits(m_glfwWindow, 700, 600, GLFW_DONT_CARE, GLFW_DONT_CARE);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

	io.Fonts->AddFontFromFileTTF("fonts/JetBrainsMono-Regular.ttf", 20);

    ImGuiStyle &style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

	style.FrameRounding = 6.0f;
	style.GrabRounding = 4.0f;
	style.WindowPadding = ImVec2(18.0f, 18.0f);
	style.FramePadding = ImVec2(10.0f, 7.0f);
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);

	ImVec4 *colors = style.Colors;
	colors[ImGuiCol_Text]                   = ImVec4(0.76f, 0.79f, 0.95f, 1.00f);
	colors[ImGuiCol_TextDisabled]           = ImVec4(0.33f, 0.35f, 0.48f, 1.00f);
	colors[ImGuiCol_WindowBg]               = ImVec4(0.10f, 0.11f, 0.15f, 1.00f);
	colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border]                 = ImVec4(0.71f, 0.61f, 0.95f, 0.25f);
	colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg]                = ImVec4(0.50f, 0.35f, 0.84f, 0.14f);
	colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.50f, 0.35f, 0.84f, 0.25f);
	colors[ImGuiCol_FrameBgActive]          = ImVec4(0.53f, 0.37f, 0.86f, 0.31f);
	colors[ImGuiCol_TitleBg]                = ImVec4(0.10f, 0.11f, 0.15f, 1.00f);
	colors[ImGuiCol_TitleBgActive]          = ImVec4(0.10f, 0.11f, 0.15f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_CheckMark]              = ImVec4(0.53f, 0.37f, 0.86f, 1.00f);
	colors[ImGuiCol_SliderGrab]             = ImVec4(0.53f, 0.37f, 0.86f, 1.00f);
	colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.59f, 0.42f, 0.97f, 1.00f);
	colors[ImGuiCol_Button]                 = ImVec4(0.53f, 0.37f, 0.86f, 1.00f);
	colors[ImGuiCol_ButtonHovered]          = ImVec4(0.57f, 0.40f, 0.93f, 1.00f);
	colors[ImGuiCol_ButtonActive]           = ImVec4(0.59f, 0.42f, 0.97f, 1.00f);
	colors[ImGuiCol_Header]                 = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_HeaderHovered]          = ImVec4(0.57f, 0.40f, 0.93f, 1.00f);
	colors[ImGuiCol_HeaderActive]           = ImVec4(0.59f, 0.42f, 0.97f, 1.00f);
	colors[ImGuiCol_Separator]              = ImVec4(0.71f, 0.61f, 0.95f, 0.79f);
	colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
	colors[ImGuiCol_SeparatorActive]        = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
	colors[ImGuiCol_ResizeGrip]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.57f, 0.40f, 0.93f, 1.00f);
	colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.57f, 0.40f, 0.93f, 1.00f);
	colors[ImGuiCol_TabHovered]             = ImVec4(0.08f, 0.08f, 0.11f, 1.00f);
	colors[ImGuiCol_Tab]                    = ImVec4(0.10f, 0.11f, 0.15f, 1.00f);
	colors[ImGuiCol_TabSelected]            = ImVec4(0.10f, 0.11f, 0.15f, 1.00f);
	colors[ImGuiCol_TabSelectedOverline]    = ImVec4(0.71f, 0.61f, 0.95f, 1.00f);
	colors[ImGuiCol_TabDimmed]              = ImVec4(0.10f, 0.11f, 0.15f, 1.00f);
	colors[ImGuiCol_TabDimmedSelected]      = ImVec4(0.10f, 0.11f, 0.15f, 1.00f);
	colors[ImGuiCol_TabDimmedSelectedOverline]  = ImVec4(0.71f, 0.61f, 0.95f, 1.00f);
	colors[ImGuiCol_DockingPreview]         = ImVec4(0.53f, 0.37f, 0.86f, 1.00f);
	colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
	colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
	colors[ImGuiCol_TableBorderLight]       = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
	colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
	colors[ImGuiCol_TextLink]               = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

	m_vkContext.Initialize(m_glfwWindow);
	Resources::Initialize(&m_vkContext);

	AddWindowPlugin(WizardWindow::Create());
	AddWindowPlugin(EditorWindow::Create(std::filesystem::path("untitled.txt")));
	m_lastWindow = m_windows[1];

    while (!glfwWindowShouldClose(m_glfwWindow))
    {
        glfwPollEvents();

		m_vkContext.BeginFrame();

		ImGui::NewFrame();
		ImGui::DockSpaceOverViewport();

		//if (ImGui::IsKeyPressed(ImGuiKey_F5)) system("cd ../../ && build.bat");
		//if (ImGui::IsKeyPressed(ImGuiKey_F6)) system("cd ../../ && run.bat");

		RenderAllWindows();
		ManageFreedCache();

		ImGui::Render();
		m_vkContext.EndFrame();
    }

	Resources::Shutdown();
	m_vkContext.Shutdown();
	ImGui::DestroyContext();

    glfwDestroyWindow(m_glfwWindow);
    glfwTerminate();
}

void Application::AddWindowPlugin(std::shared_ptr<WindowPlugin> window)
{
	window->m_application = this;
	m_windows.push_back(std::move(window));
}

void Application::ManageFreedCache(void)
{
	if (m_freedWindowIndices.empty())
		return;
	
	for (uint32_t index : m_freedWindowIndices)
		m_windows.erase(m_windows.begin() + index);
	
	m_freedWindowIndices.clear();
}

void Application::RenderAllWindows(void)
{
	for (uint32_t i = 0; i < m_windows.size(); i++)
	{
		std::shared_ptr<WindowPlugin> &window = m_windows[i];
		if (!window)
			continue;

		if (!window->m_opened)
			m_freedWindowIndices.push_back(i);

		window->RenderWindow();
	}
}

bool Application::CheckForWindowWithPath(std::filesystem::path &path)
{
	for (std::shared_ptr<WindowPlugin> window : m_windows)
	{
		std::shared_ptr<EditorWindow> editorWindow = std::dynamic_pointer_cast<EditorWindow>(window);

		if (!editorWindow)
			continue;

		if (editorWindow->GetFilePath().string() == path.string())
			return true;
	}
	return false;
}

ImVec2 Application::GetGlfwWindowPosition(void)
{
	int32_t x, y;
	glfwGetWindowPos(m_glfwWindow, &x, &y);
	return ImVec2(x, y);
}

ImVec2 Application::GetGlfwWindowSize(void)
{
	int32_t x, y;
	glfwGetWindowSize(m_glfwWindow, &x, &y);
	return ImVec2(x, y);
}

};