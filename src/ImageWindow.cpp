#include "ImageWindow.h"
#include "Application.h"
#include "Resources.h"
#include "Math/Math.h"

#include <iostream>

namespace Crystal
{

ImageWindow::ImageWindow(const std::filesystem::path &filePath)
{
	m_filePath = filePath;
}

ImageWindow::~ImageWindow(void)
{
	m_application->GetRenderer().DestroyTexture(m_texture);
}

void ImageWindow::OnWindowAdded(void)
{
	m_texture = m_application->GetRenderer().CreateTexture(m_filePath.string().c_str());
}

void ImageWindow::RenderWindow(void)
{
	std::string title = m_filePath.filename().string() + "##" + m_filePath.string();
	const char *c_title = title.c_str();

	static bool isDragging = false;
	static ImVec2 dragStart;
	
	ImGuiIO &io = ImGui::GetIO();
	m_smoothZoomFactor = Math::Lerp(m_smoothZoomFactor, m_zoomFactor, io.DeltaTime * 15.0f);

	LayoutHandler &layoutHandler = m_application->GetLayoutHandler();
    ImGui::SetNextWindowDockID(layoutHandler.GetMainDockID(), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(c_title, &m_opened, ImGuiWindowFlags_NoScrollWithMouse))
    {
        ImVec2 windowSize = ImGui::GetContentRegionAvail();

        ImVec2 imageSize = ImVec2(m_texture->width, m_texture->height) * m_smoothZoomFactor * windowSize.x / m_texture->width;
        ImVec2 imagePos = (windowSize - imageSize) * 0.5f  + m_offset;
        
        const bool isHovered = ImGui::IsWindowHovered();

        if (ImGui::IsWindowFocused())
        {
		    WindowManager &wm = m_application->GetWindowManager();
            wm.SetLastEditorWindow(this);

            if (io.KeyCtrl)
				if (ImGui::IsKeyPressed(ImGuiKey_Equal)) m_zoomFactor *= 1.2f;
				else if (ImGui::IsKeyPressed(ImGuiKey_Minus)) m_zoomFactor *= 0.8f;

            if (isHovered && ImGui::IsWindowDocked() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
            {
                ImVec2 mouseDragDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
                m_offset += mouseDragDelta;
                ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
            }
        }

        if (isHovered)
        {
            float wheel = io.MouseWheel;
            if (wheel > 0)
                m_zoomFactor *= 1.1f;
            else if (wheel < 0)
                m_zoomFactor *= 0.9f;
        }

        if (m_zoomFactor < 0.1f)
            m_zoomFactor = 0.1f;

        ImGui::SetCursorPos(imagePos);
        ImGui::Image((ImTextureID)m_texture->id, imageSize);
    }
    ImGui::End();
}

}