#include "Application.h"
#include "Window.h"
#include "EditorWindow.h"
#include "ImageWindow.h"
#include "PreferencesWindow.h"
#include "Resources.h"
#include "Rendering/VulkanRenderer.h"

#include "imgui.h"
#include "imgui_internal.h"

#include "ImGuiFileDialog.h"
#include "ExplorerWindow.h"
#include "TerminalWindow.h"
#include "Utils.h"

namespace Crystal
{
static bool isMaximizedPressed = false, isMinimizedPressed = false;

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
			if (io.KeyShift)
				m_windowManager.ForEachWindowOfType<EditorWindow>([&](EditorWindow *window){
					window->SaveToFile();
					std::cout << window->GetFilePath() << '\n';
				});
			else
			{
				EditorWindow *window = dynamic_cast<EditorWindow*>(m_windowManager.GetLastEditorWindow());
				if (window) window->SaveToFile();
			}
		}
		else if (io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_E, false))
		{
			if (!m_windowManager.CheckForWindowOfType<ExplorerWindow>())
				m_windowManager.AddWindow(new ExplorerWindow());
		}
		else if (io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_T, false))
		{
			m_windowManager.AddWindow(new TerminalWindow());
		}
	}

	const float iconSize = ImGui::GetFontSize();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0, iconSize * 0.5f });
	if (ImGui::BeginMainMenuBar())
	{
		ImDrawList *drawList = ImGui::GetWindowDrawList();
		ImGuiWindow* window = ImGui::GetCurrentWindow();

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
				EditorWindow *window = dynamic_cast<EditorWindow*>(m_windowManager.GetLastEditorWindow());
				if (window) window->SaveToFile();
			}
			if (ImGui::MenuItem("Save All", "Ctrl+Shift+S"))
			{
				m_windowManager.ForEachWindowOfType<EditorWindow>([&](EditorWindow *window){
					window->SaveToFile();
				});
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Preferences"))
			{
				if (!m_windowManager.CheckForWindowOfType<PreferencesWindow>())
					m_windowManager.AddWindow(new PreferencesWindow());
				else
					ImGui::SetWindowFocus("Preferences##Preferences");
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Explorer", "Ctrl+Shift+E"))
			{
				if (!m_windowManager.CheckForWindowOfType<ExplorerWindow>())
					m_windowManager.AddWindow(new ExplorerWindow());
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Terminal"))
		{
			if (ImGui::MenuItem("New Terminal", "Ctrl+Shift+T"))
				m_windowManager.AddWindow(new TerminalWindow());
			ImGui::EndMenu();
		}
		/*if (ImGui::BeginMenu("Build"))
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
		}*/

	// Draw the minimize, maximize and close buttons
	{
		ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });
		ImGui::SameLine(ImGui::GetWindowWidth() - iconSize * 6.0f - 18.0f);

		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImVec2 center = { pos.x + iconSize, pos.y + iconSize };

		if (ImGui::Button("##MinimizeBtn", { iconSize * 2.0f, iconSize * 2.0f }))
		{
			isMinimizedPressed = true;
			m_mainWindow->Minimize();
		}
		else
			isMinimizedPressed = false;

		drawList->AddLine({ center.x - iconSize * 0.25f, center.y }, { center.x + iconSize * 0.25f, center.y }, ImGui::GetColorU32(ImGuiCol_Text), 1.0f);

		ImGui::SameLine();
		pos = ImGui::GetCursorScreenPos();
		center = { pos.x + iconSize, pos.y + iconSize };

		if (ImGui::Button("##MaximizeBtn", { iconSize * 2.0f, iconSize * 2.0f }))
		{
			isMaximizedPressed = true;
			m_mainWindow->Maximize();
		}
		else
			isMaximizedPressed = false;

		if (m_mainWindow->IsMaximized())
		{
			center -= { -iconSize * 0.03f, iconSize * 0.05f };

			drawList->AddLine({ center.x - iconSize * 0.2f, center.y - iconSize * 0.2f }, { center.x + iconSize * 0.2f, center.y - iconSize * 0.2f }, ImGui::GetColorU32(ImGuiCol_Text), 1.0f);
			drawList->AddLine({ center.x + iconSize * 0.2f, center.y - iconSize * 0.2f }, { center.x + iconSize * 0.2f, center.y + iconSize * 0.2f }, ImGui::GetColorU32(ImGuiCol_Text), 1.0f);

			center += { -iconSize * 0.07f, iconSize * 0.1f };

			drawList->AddRect({ center.x - iconSize * 0.2f, center.y - iconSize * 0.2f }, { center.x + iconSize * 0.2f, center.y + iconSize * 0.2f }, ImGui::GetColorU32(ImGuiCol_Text), 0.0f, 0, 1.0f);
		}
		else
		{
			drawList->AddRect({ center.x - iconSize * 0.25f, center.y - iconSize * 0.25f }, { center.x + iconSize * 0.25f, center.y + iconSize * 0.25f }, ImGui::GetColorU32(ImGuiCol_Text), 0.0f, 0, 1.0f);
		}

		ImGui::SameLine();
		pos = ImGui::GetCursorScreenPos();
		center = { pos.x + iconSize, pos.y + iconSize };

		if (ImGui::Button("##CloseBtn", { iconSize * 2.0f, iconSize * 2.0f }))
			m_mainWindow->Close();

		drawList->AddLine({ center.x - iconSize * 0.25f, center.y - iconSize * 0.25f }, { center.x + iconSize * 0.25f, center.y + iconSize * 0.25f }, ImGui::GetColorU32(ImGuiCol_Text), 1.0f);
		drawList->AddLine({ center.x - iconSize * 0.25f, center.y + iconSize * 0.25f }, { center.x + iconSize * 0.25f, center.y - iconSize * 0.25f }, ImGui::GetColorU32(ImGuiCol_Text), 1.0f);

		ImGui::PopStyleVar();
		ImGui::PopStyleColor();

		ImVec2 p1 = ImVec2(window->Pos.x, window->Pos.y + window->Size.y - 1);
		ImVec2 p2 = ImVec2(window->Pos.x + window->Size.x, window->Pos.y + window->Size.y - 1);
		drawList->AddLine(p1, p2, ImColor(style.Colors[ImGuiCol_Border]), (int)style.WindowBorderSize);
	}

#if CRYSTAL_PLATFORM_WINDOWS
		m_mainWindow->SetCaptionHeight(ImGui::IsAnyItemHovered() ? 0 : iconSize * 2.0f);
#endif
		ImGui::EndMainMenuBar();
	}
	ImGui::PopStyleVar(2);

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
			{
				SetMainDirectoryPath(path);
				if (!m_windowManager.CheckForWindowOfType<ExplorerWindow>())
					m_windowManager.AddWindow(new ExplorerWindow());
			}
		}
		
		ImGuiFileDialog::Instance()->Close();
	}

	ImGui::PopStyleVar();
	ImGui::PopStyleColor();

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
	if (ImGui::BeginViewportSideBar("##BottomStatusBar", NULL, ImGuiDir_Down, style.FramePadding.y * 2 + iconSize - 1, window_flags))
	{
		if (ImGui::BeginMenuBar())
		{
			EditorWindow *last = dynamic_cast<EditorWindow*>(m_windowManager.GetLastEditorWindow());
			if (last)
			{
				TextEditor *editor = last->GetTextEditor();
				int32_t line, column;
				editor->GetCursorPosition(line, column);
				ImGui::Text("%s", editor->GetLanguageDefinitionName());
				ImGui::SameLine(); ImGui::Dummy(ImVec2(16, 0));
				ImGui::Text("Tab Size: %d", editor->GetTabSize());
				ImGui::SameLine(); ImGui::Dummy(ImVec2(16, 0));
				ImGui::Text("Ln %d, Col %d", line, column);
			}
			ImGui::EndMenuBar();
		}
	}
	ImGui::End();

static bool show_command_palette = true;
	//ImCmd::CommandPaletteWindow("CommandPalette", &show_command_palette);

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
		[&]() { if (isMaximizedPressed || isMinimizedPressed) return; OnRender(); },
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
	style.FramePadding = ImVec2(20.0f, 8.0f);
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_Right;

	m_renderer = new VulkanRenderer(m_mainWindow);
	Resources::Initialize(m_renderer);

	m_windowManager = WindowManager(this);
	m_dragDropHandler = DragDropHandler(this);

	m_preferences = std::make_unique<Preferences>(m_mainWindow, m_windowManager);

	while (m_mainWindow->IsRunning())
	{
		m_mainWindow->PollEvents();

		OnRender();

		m_preferences->Refresh();
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
	if (extension == ".png" || extension == ".jpg" ||
		extension == ".psd" || extension == ".gif")
		m_windowManager.AddWindow(new ImageWindow(path));
	else
		m_windowManager.AddWindow(new EditorWindow(path));
}

}