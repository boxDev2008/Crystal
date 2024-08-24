#include "ImageWindow.h"
#include "Application.h"
#include "Resources.h"
#include "math/Lerp.h"

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
	static Vector2 dragStart;
	
	ImGuiIO &io = ImGui::GetIO();
	m_smoothZoomFactor = Math::lerp(m_smoothZoomFactor, m_zoomFactor, io.DeltaTime * 15.0f);

    ImGui::SetNextWindowDockID(m_application->GetLayoutHandler().GetMainDockID(), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(c_title, &m_opened, ImGuiWindowFlags_NoScrollWithMouse))
    {
        Vector2 windowSize = ImGui::GetContentRegionAvail();

        Vector2 imageSize = Vector2(m_texture->width, m_texture->height) * m_smoothZoomFactor * windowSize.x / m_texture->width;
        Vector2 imagePos = (windowSize - imageSize) * 0.5f  + m_offset;
        
        const bool isHovered = ImGui::IsWindowHovered();

        if (ImGui::IsWindowFocused())
        {
            m_application->GetWindowManager().SetLastWindow(this);

            if (io.KeyCtrl)
				if (ImGui::IsKeyPressed(ImGuiKey_Equal)) m_zoomFactor *= 1.2f;
				else if (ImGui::IsKeyPressed(ImGuiKey_Minus)) m_zoomFactor *= 0.8f;

            if (isHovered && ImGui::IsWindowDocked() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
            {
                Vector2 mouseDragDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
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