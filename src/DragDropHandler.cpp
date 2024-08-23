#include "DragDropHandler.h"
#include "Application.h"
#include "EditorWindow.h"
#include "ImageWindow.h"
#include "imgui.h"

namespace Crystal
{

namespace fs = std::filesystem;

void DragDropHandler::BeginDragDropFileMoveTarget(const std::filesystem::path &path)
{
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH"))
        {
            const char* payloadPath = (const char*)payload->Data;
            fs::path srcPath(payloadPath);
            fs::path destPath = path / srcPath.filename();
            WindowManager &wm = m_application->GetWindowManager();
            if (EditorWindow *editorWindow = dynamic_cast<EditorWindow*>(wm.CheckForWindowWithPath(srcPath)))
                editorWindow->SetFilePath(fs::path(destPath.string()));
            else if (ImageWindow *imageWindow = dynamic_cast<ImageWindow*>(wm.CheckForWindowWithPath(srcPath)))
                imageWindow->SetFilePath(fs::path(destPath.string()));
            try {
                fs::rename(srcPath, destPath);
            }
            catch (const std::filesystem::filesystem_error& e) {
                std::cerr << "Error renaming file: " << e.what() << '\n';
            }
        }
        else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DIRECTORY_PATH"))
        {
            const char* payloadPath = (const char*)payload->Data;
            fs::path srcPath(payloadPath);
            fs::path destPath = path / srcPath.filename();
            try {
                fs::rename(srcPath, destPath);
            }
            catch (const std::filesystem::filesystem_error& e) {
                std::cerr << "Error renaming file: " << e.what() << '\n';
            }
        }
        ImGui::EndDragDropTarget();
    }
}

void DragDropHandler::BeginDragDropFileOpenTarget(void)
{
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH"))
        {
            const char* payloadPath = (const char*)payload->Data;
            m_application->OpenFile(payloadPath);
        }
        ImGui::EndDragDropTarget();
    }
}

}
