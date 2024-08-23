#pragma once

#include "imgui.h"

namespace Crystal
{

class LayoutHandler
{
public:
    void BuildDockspaceLayout(void);

    const ImGuiID GetLeftDockID(void) const { return m_leftDockId; }
    const ImGuiID GetRightDockID(void) const { return m_rightDockId; }
    const ImGuiID GetBottomDockID(void) const { return m_bottomDockId; }
    const ImGuiID GetMainDockID(void) const { return m_mainDockId; }

private:
    ImGuiID m_leftDockId = -1, m_rightDockId = -1 , m_bottomDockId = -1, m_mainDockId = -1;
};

}