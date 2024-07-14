#include "ExplorerWindow.h"
#include "EditorWindow.h"
#include "Application.h"

namespace Crystal
{

std::shared_ptr<ExplorerWindow> ExplorerWindow::Create(std::filesystem::path &directory)
{
	std::shared_ptr<ExplorerWindow> window = std::make_shared<ExplorerWindow>();
	window->m_mainDirectoryPath = directory;
	return window;
}

void ExplorerWindow::RenderBranch(std::filesystem::path &directory)
{
	namespace fs = std::filesystem;
	if (fs::exists(directory) && fs::is_directory(directory))
	{
        for (const auto &entry : fs::directory_iterator(directory))
        {
            fs::path path = entry.path();

            if (fs::is_directory(path))
            {
                if (ImGui::TreeNodeEx(path.filename().string().c_str()))
                {
                    ExplorerWindow::RenderBranch(path);
                    ImGui::TreePop();
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
					if (ImGui::IsItemHovered() && (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) || ImGui::IsKeyPressed(ImGuiKey_Enter, false)))
					{
						if (!m_application->CheckForWindowWithPath(path))
							m_application->AddWindowPlugin(EditorWindow::Create(path));
						else
							ImGui::SetWindowFocus(path.filename().string().c_str());
					}
                }
            }
        }
    }
	else
        std::cerr << "The provided path is not a directory or does not exist." << std::endl;
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