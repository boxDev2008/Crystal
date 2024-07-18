#include "ExplorerWindow.h"
#include "EditorWindow.h"
#include "Application.h"

#include <fstream>

#if _WIN32
#include <windows.h>
#endif

namespace Crystal
{

std::shared_ptr<ExplorerWindow> ExplorerWindow::Create(const std::filesystem::path &directory)
{
	std::shared_ptr<ExplorerWindow> window = std::make_shared<ExplorerWindow>();
	window->m_mainDirectoryPath = directory;
	return window;
}

static std::filesystem::path s_selectedPath;
void ExplorerWindow::RenderBranch(std::filesystem::path &directory)
{
	namespace fs = std::filesystem;

    if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    {
        s_selectedPath = m_mainDirectoryPath;
        ImGui::OpenPopup("##ExplorerMainDirectoryPopup");
    }

	if (fs::exists(directory) && fs::is_directory(directory))
	{
        for (const auto &entry : fs::directory_iterator(directory))
        {
            fs::path path = entry.path();

            if (fs::is_directory(path))
            {
                if (ImGui::TreeNodeEx(path.filename().string().c_str()))
                {
                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                    {
                        s_selectedPath = path;
                        ImGui::OpenPopup("##ExplorerDirectoryPopup");
                    }
                    ExplorerWindow::RenderBranch(path);
                    ImGui::TreePop();
                }
                else if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                {
                    s_selectedPath = path;
                    ImGui::OpenPopup("##ExplorerDirectoryPopup");
                }
            }
        }

        for (const auto &entry : fs::directory_iterator(directory))
        {
            fs::path path = entry.path();

            if (fs::is_regular_file(path))
            {
                if (ImGui::TreeNodeEx(path.filename().string().c_str(), ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf))
                {
                    // Load file (create a window for it if one doesn't exist)
					if (ImGui::IsItemHovered())
					{
                        if ((ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) || (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter, false))))
                        {
                            if (!m_application->CheckForWindowWithPath(path))
                                m_application->AddWindowPlugin(EditorWindow::Create(path));
                            else
                                ImGui::SetWindowFocus(path.filename().string().c_str());
                        }
                        else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                        {
                            s_selectedPath = path;
                            ImGui::OpenPopup("##ExplorerFilePopup");
                        }
					}
                }
            }
        }
    }
	else
        std::cerr << "The provided path is not a directory or does not exist." << std::endl;

    static bool confirmDeletePopupOpened = false;
    static bool createFilePopupOpened = false;
    static bool createDirectoryPopupOpened = false;
    static bool renameItemPopupOpened = false;

    if (ImGui::BeginPopup("##ExplorerFilePopup"))
    {
#if _WIN32
        if (ImGui::MenuItem("View in File Explorer")) { ShellExecuteA(NULL, "open", s_selectedPath.remove_filename().string().c_str(), NULL, NULL, SW_SHOWDEFAULT); }
#endif
        if (ImGui::MenuItem("Rename"))
            renameItemPopupOpened = true;

        if (ImGui::MenuItem("Delete"))
            confirmDeletePopupOpened = true;

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("##ExplorerDirectoryPopup"))
    {
        if (ImGui::MenuItem("New File"))
            createFilePopupOpened = true;

        if (ImGui::MenuItem("New Folder"))
            createDirectoryPopupOpened = true;

#if _WIN32
        if (ImGui::MenuItem("View in File Explorer")) { ShellExecuteA(NULL, "open", s_selectedPath.remove_filename().string().c_str(), NULL, NULL, SW_SHOWDEFAULT); }
#endif
        if (ImGui::MenuItem("Rename"))
            renameItemPopupOpened = true;

        if (ImGui::MenuItem("Delete"))
            confirmDeletePopupOpened = true;

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("##ExplorerMainDirectoryPopup"))
    {
        if (ImGui::MenuItem("New File"))
            createFilePopupOpened = true;

        if (ImGui::MenuItem("New Folder"))
            createDirectoryPopupOpened = true;

#if _WIN32
        if (ImGui::MenuItem("View in File Explorer")) { ShellExecuteA(NULL, "open", s_selectedPath.remove_filename().string().c_str(), NULL, NULL, SW_SHOWDEFAULT); }
#endif

        ImGui::EndPopup();
    }

    static char inputText[64];

    if (confirmDeletePopupOpened)
    {
        ImVec2 mouseCurosrPosition = m_application->GetMouseCursorPosition();
        ImVec2 mainWindowPosition = m_application->GetGlfwWindowPosition();
        ImGui::SetWindowPos("##ConfirmDeletePopup", ImVec2(mouseCurosrPosition.x + mainWindowPosition.x, mouseCurosrPosition.y + mainWindowPosition.y));
        ImGui::OpenPopup("##ConfirmDeletePopup");
        confirmDeletePopupOpened = false;
    }

    if (createFilePopupOpened)
    {
        ImVec2 mouseCurosrPosition = m_application->GetMouseCursorPosition();
        ImVec2 mainWindowPosition = m_application->GetGlfwWindowPosition();
        ImGui::SetWindowPos("##CreateFilePopupOpened", ImVec2(mouseCurosrPosition.x + mainWindowPosition.x, mouseCurosrPosition.y + mainWindowPosition.y));
        ImGui::OpenPopup("##CreateFilePopupOpened");
        memset(inputText, 0, sizeof(inputText));
        createFilePopupOpened = false;
    }

    if (createDirectoryPopupOpened)
    {
        ImVec2 mouseCurosrPosition = m_application->GetMouseCursorPosition();
        ImVec2 mainWindowPosition = m_application->GetGlfwWindowPosition();
        ImGui::SetWindowPos("##CreateDirectoryPopupOpened", ImVec2(mouseCurosrPosition.x + mainWindowPosition.x, mouseCurosrPosition.y + mainWindowPosition.y));
        ImGui::OpenPopup("##CreateDirectoryPopupOpened");
        memset(inputText, 0, sizeof(inputText));
        createDirectoryPopupOpened = false;
    }

    if (renameItemPopupOpened)
    {
        ImVec2 mouseCurosrPosition = m_application->GetMouseCursorPosition();
        ImVec2 mainWindowPosition = m_application->GetGlfwWindowPosition();
        ImGui::SetWindowPos("##RenameItemPopupOpened", ImVec2(mouseCurosrPosition.x + mainWindowPosition.x, mouseCurosrPosition.y + mainWindowPosition.y));
        ImGui::OpenPopup("##RenameItemPopupOpened");
        memcpy(inputText, s_selectedPath.filename().string().c_str(), sizeof(inputText));
        renameItemPopupOpened = false;
    }

    if (ImGui::BeginPopupModal("##ConfirmDeletePopup", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImVec2 mouseCurosrPosition = m_application->GetMouseCursorPosition();
        ImVec2 mainWindowPosition = m_application->GetGlfwWindowPosition();
        ImGui::SetWindowPos(ImVec2(mouseCurosrPosition.x + mainWindowPosition.x, mouseCurosrPosition.y + mainWindowPosition.y), ImGuiCond_Once);

        const std::string title = "Are you sure you want to delete " + s_selectedPath.filename().string() + "?";
        ImGui::Text(title.c_str());

        if (!ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsKeyPressed(ImGuiKey_Escape, false))
            ImGui::CloseCurrentPopup();

        if (ImGui::Button("Yes"))
        {
            std::filesystem::remove_all(s_selectedPath);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Never mind"))
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("##CreateFilePopupOpened", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImVec2 mouseCurosrPosition = m_application->GetMouseCursorPosition();
        ImVec2 mainWindowPosition = m_application->GetGlfwWindowPosition();
        ImGui::SetWindowPos(ImVec2(mouseCurosrPosition.x + mainWindowPosition.x, mouseCurosrPosition.y + mainWindowPosition.y), ImGuiCond_Once);

        if (!ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsKeyPressed(ImGuiKey_Escape, false))
            ImGui::CloseCurrentPopup();

        if (!ImGui::IsAnyItemActive()) 
            ImGui::SetKeyboardFocusHere();

        ImGui::InputText("Enter Name", inputText, IM_ARRAYSIZE(inputText));

        if (ImGui::Button("Create") || ImGui::IsKeyPressed(ImGuiKey_Enter, false))
        {
            std::ofstream file(s_selectedPath.string() + '\\' + inputText);
			file.close();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("##CreateDirectoryPopupOpened", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImVec2 mouseCurosrPosition = m_application->GetMouseCursorPosition();
        ImVec2 mainWindowPosition = m_application->GetGlfwWindowPosition();
        ImGui::SetWindowPos(ImVec2(mouseCurosrPosition.x + mainWindowPosition.x, mouseCurosrPosition.y + mainWindowPosition.y), ImGuiCond_Once);

        if (!ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsKeyPressed(ImGuiKey_Escape, false))
            ImGui::CloseCurrentPopup();

        if (!ImGui::IsAnyItemActive()) 
            ImGui::SetKeyboardFocusHere();

        ImGui::InputText("Enter Name", inputText, IM_ARRAYSIZE(inputText));

        if (ImGui::Button("Create") || ImGui::IsKeyPressed(ImGuiKey_Enter, false))
        {
            std::filesystem::create_directory(s_selectedPath.string() + '\\' + inputText);
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("##RenameItemPopupOpened", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImVec2 mouseCurosrPosition = m_application->GetMouseCursorPosition();
        ImVec2 mainWindowPosition = m_application->GetGlfwWindowPosition();
        ImGui::SetWindowPos(ImVec2(mouseCurosrPosition.x + mainWindowPosition.x, mouseCurosrPosition.y + mainWindowPosition.y), ImGuiCond_Once);

        if (!ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsKeyPressed(ImGuiKey_Escape, false))
            ImGui::CloseCurrentPopup();

        if (!ImGui::IsAnyItemActive()) 
            ImGui::SetKeyboardFocusHere();

        ImGui::InputText("Enter Name", inputText, IM_ARRAYSIZE(inputText));

        if (ImGui::Button("Rename") || ImGui::IsKeyPressed(ImGuiKey_Enter, false))
        {
            if (std::shared_ptr<EditorWindow> editorWindow = std::dynamic_pointer_cast<EditorWindow>(m_application->CheckForWindowWithPath(s_selectedPath)))
                editorWindow->SetFilePath(fs::path(s_selectedPath.parent_path().string() + '\\' + inputText));

            std::filesystem::rename(s_selectedPath, s_selectedPath.parent_path().string() + '\\' + inputText);
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void ExplorerWindow::RenderWindow(void)
{
	if (ImGui::Begin("Explorer"))
	{
		RenderBranch(m_mainDirectoryPath);
		ImGui::End();
	}
}

}