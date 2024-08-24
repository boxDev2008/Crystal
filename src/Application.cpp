#include "Application.h"
#include "Window.h"
#include "WizardWindow.h"
#include "EditorWindow.h"
#include "ImageWindow.h"
#include "PreferencesWindow.h"
#include "Resources.h"
#include "rendering/VulkanRenderer.h"

#include "imgui.h"
#include "imgui_internal.h"

#include "ImGuiFileDialog.h"
#include "ExplorerWindow.h"
#include "TerminalWindow.h"
#include "Utils.h"
#include <filesystem>

#include <algorithm>
#include <sstream>

namespace Crystal
{

using namespace Math;

void Application::OnRender(void)
{
	ImGuiIO &io = ImGui::GetIO();
	ImGuiStyle &style = ImGui::GetStyle();

	m_renderer->BeginFrame();

	ImGui::NewFrame();
	m_layoutHandler.BuildDockspaceLayout();

	if (io.KeyCtrl)
	{
		if (ImGui::IsKeyPressed(ImGuiKey_O, false))
		{
			IGFD::FileDialogConfig config;
			config.flags = ImGuiFileDialogFlags_Modal;
			config.path = ".";
			config.countSelectionMax = 1;
			ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".*", config);
		}
		else if (ImGui::IsKeyPressed(ImGuiKey_K, false))
		{
			IGFD::FileDialogConfig config;
			config.flags = ImGuiFileDialogFlags_Modal;
			config.path = ".";
			config.countSelectionMax = 1;
			ImGuiFileDialog::Instance()->OpenDialog("ChooseDirectoryDlgKey", "Choose Directory", nullptr, config);
		}
		else if (ImGui::IsKeyPressed(ImGuiKey_S, false))
		{
			EditorWindow *window = dynamic_cast<EditorWindow*>(m_windowManager.GetLastWindow());
			if (window) window->SaveToFile();
		}
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open File", "Ctrl+O"))
			{ 
				IGFD::FileDialogConfig config;
				config.flags = ImGuiFileDialogFlags_Modal;
				config.path = ".";
				config.countSelectionMax = 1;
				ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".*", config);
			}
			if (ImGui::MenuItem("Open Directroy", "Ctrl+K"))
			{ 
				IGFD::FileDialogConfig config;
				config.flags = ImGuiFileDialogFlags_Modal;
				config.path = ".";
				config.countSelectionMax = 1;
				ImGuiFileDialog::Instance()->OpenDialog("ChooseDirectoryDlgKey", "Choose Directory", nullptr, config);
			}
			if (ImGui::MenuItem("Save", "Ctrl+S"))
			{
				EditorWindow *window = dynamic_cast<EditorWindow*>(m_windowManager.GetLastWindow());
				if (window) window->SaveToFile();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Preferences"))
			{
				if (!m_windowManager.CheckForWindowOfType<PreferencesWindow>())
					m_windowManager.AddWindow(new PreferencesWindow());
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Terminal"))
		{
			if (ImGui::MenuItem("New Terminal"))
				m_windowManager.AddWindow(new TerminalWindow());
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Build"))
		{
			if (ImGui::MenuItem("Build and Run"))
			{

			}
			if (ImGui::MenuItem("Build"))
			{

			}
			if (ImGui::MenuItem("Run"))
			{

			}
			ImGui::EndMenu();
		}

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImGuiWindow* window = ImGui::GetCurrentWindow();

		Vector2 p1 = Vector2(window->Pos.x, window->Pos.y + window->Size.y - 1);
		Vector2 p2 = Vector2(window->Pos.x + window->Size.x, window->Pos.y + window->Size.y - 1);

		drawList->AddLine(p1, p2, ImColor(style.Colors[ImGuiCol_Border]), style.WindowBorderSize);

		ImGui::EndMainMenuBar();
	}
	ImGui::PopStyleVar();

	ImGui::PushStyleColor(ImGuiCol_PopupBg, style.Colors[ImGuiCol_WindowBg]);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 10.0f, 5.0f });

	ImGui::SetNextWindowSize({ 1000, 700 }, ImGuiCond_FirstUseEver);
	if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", 32, { 200, 100 }))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			std::string path = ImGuiFileDialog::Instance()->GetFilePathName();
			OpenFile(path);
		}
		
		ImGuiFileDialog::Instance()->Close();
	}

	ImGui::SetNextWindowSize({ 1000, 700 }, ImGuiCond_FirstUseEver);
	if (ImGuiFileDialog::Instance()->Display("ChooseDirectoryDlgKey", 32, { 200, 100 }))
	{
		if (ImGui::IsKeyPressed(ImGuiKey_Escape, false))
			ImGui::CloseCurrentPopup();

		if (ImGuiFileDialog::Instance()->IsOk())
		{
			std::string path = ImGuiFileDialog::Instance()->GetCurrentPath();
			if (path != GetMainDirectoryPath())
				SetMainDirectoryPath(path);
		}
		
		ImGuiFileDialog::Instance()->Close();
	}

	ImGui::PopStyleVar();
	ImGui::PopStyleColor();

	m_windowManager.RenderAllWindows();

	ImGui::Render();
	m_renderer->EndFrame();
}

Application::Application(void)
{
	glfwSetErrorCallback([](int error, const char* description) {
		fprintf(stderr, "GLFW Error %d: %s\n", error, description);
	});

	PlatformWindowSettings settings = PlatformWindowSettings::Default();
	m_mainWindow = new PlatformWindow(settings,
		[&]() { OnRender(); },
		[&](int32_t count, const char **paths) {
		if (count == 1 && !std::filesystem::is_regular_file(paths[0]) && paths[0] != GetMainDirectoryPath())
		{
			const char *path = paths[0];
			SetMainDirectoryPath(path);		
			return;
		}

		for (int32_t i = 0; i < count; ++i)
		{
			const char *path = paths[i];

			if (!std::filesystem::is_regular_file(path))
				continue;

			if (GetWindowManager().CheckForWindowWithPath(path))
				continue;

			OpenFile(path);
		}
	});

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;

	io.IniFilename = nullptr;
          
	ImGuiStyle &style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	style.FrameRounding = 2.0f;
	style.GrabRounding = 1.0f;
	style.ScrollbarRounding = 0.0f;
	style.TabRounding = 0.0f;
	style.DockingSeparatorSize = 1.0f;
	style.ItemSpacing = ImVec2(7.0f, 4.0f);
	style.WindowPadding = ImVec2(18.0f, 18.0f);
	style.FramePadding = ImVec2(20.0f, 5.0f);
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);

	m_renderer = new VulkanRenderer(m_mainWindow);
	Resources::Initialize(m_renderer);

	m_windowManager = WindowManager(this);
	m_dragDropHandler = DragDropHandler(this);

	m_preferences = Preferences(m_mainWindow, m_windowManager);
	Preferences::GlobalSettings &globalSettings = m_preferences.GetGlobalSettings();
	globalSettings.SetFont("Crystal Plex Mono");
	globalSettings.SetColorTheme("Crystal");

	m_windowManager.AddWindow(new ExplorerWindow());
	m_windowManager.AddWindow(new WizardWindow());

	double currentTime, lastTime;
	while (m_mainWindow->IsRunning())
	{
		currentTime = glfwGetTime(); 

		if(currentTime - lastTime < 1.0 / 120.0) 
			continue;

		lastTime = currentTime;

		glfwPollEvents();

		m_preferences.Refresh();
		OnRender();

		m_windowManager.ManageFreedCache();
	}

	m_windowManager.FreeAllWindows();

	Resources::Shutdown();
	delete m_renderer;
	ImGui::DestroyContext();

	delete m_mainWindow;
}
void Application::SetMainDirectoryPath(const std::filesystem::path &path)
{
	m_mainDirectory = path;
	m_windowManager.ForEachWindowOfType<TerminalWindow>([&](TerminalWindow *terminal) {
		terminal->SetCurrentDirectoryPath(path);
	});
}

void Application::OpenFile(const std::filesystem::path &path)
{
	if (m_windowManager.CheckForWindowWithPath(path))
	{
        std::string focusTarget = path.filename().string() + "##" + path.string();
        ImGui::SetWindowFocus(focusTarget.c_str());
		return;
	}

	std::filesystem::path extension = path.extension();
	if (extension == ".png" || extension == ".jpg")
		m_windowManager.AddWindow(new ImageWindow(path));
	else
		m_windowManager.AddWindow(new EditorWindow(path));
}

}