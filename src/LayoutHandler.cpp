#include "LayoutHandler.h"
#include "imgui_internal.h"

namespace Crystal
{

void LayoutHandler::BuildDockspaceLayout(void)
{
    m_mainDockId = ImGui::DockSpaceOverViewport();

    if (m_leftDockId != -1)
        return;

    m_leftDockId = ImGui::DockBuilderSplitNode(m_mainDockId, ImGuiDir_Left, 0.23f, NULL, &m_mainDockId);
    m_rightDockId = ImGui::DockBuilderSplitNode(m_mainDockId, ImGuiDir_Right, 0.3f, NULL, &m_mainDockId);
    m_bottomDockId = ImGui::DockBuilderSplitNode(m_mainDockId, ImGuiDir_Down, 0.4f, NULL, &m_mainDockId);
    ImGui::DockBuilderFinish(m_mainDockId);
}

}