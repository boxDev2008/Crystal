#include "CompletionMenu.h"
#include "EditorWindow.h"
#include "Application.h"

#include "imgui_internal.h"
#include <algorithm>

namespace Crystal
{

CompletionMenu::CompletionMenu(EditorWindow *parentWindow, Application *application) :
	m_parentWindow(parentWindow), m_application(application) { }

void CompletionMenu::SetCurrentWord(const std::string &word, int32_t wordStart, int32_t wordEnd)
{
    m_word = word;

    if (word.empty())
        return;

    m_wordStart = wordStart;
    m_wordEnd = wordEnd;
}

void CompletionMenu::AddCompletion(const std::string &completion, CompletionType type)
{
    m_completions[completion] = type;
}

void CompletionMenu::ClearCompletions(void)
{
    m_completions.clear();
}

bool CompletionMenu::FilterCompletions(void)
{
    if (m_word.empty() || m_completions.empty())
    {
        m_active = false;
        return false;
    }
    
    m_filteredCompletions.clear();

    for (const auto& completion : m_completions)
    {
        std::string completionLower = completion.first;
        std::transform(completionLower.begin(), completionLower.end(), completionLower.begin(), ::tolower);
        std::string wordLower = m_word;
        std::transform(wordLower.begin(), wordLower.end(), wordLower.begin(), ::tolower);

        if (completion.first == m_word)
            continue;

        if (completionLower.find(wordLower) != std::string::npos)
            m_filteredCompletions[completion.first] = completion.second;
    }

    if (m_filteredCompletions.empty())
    {
        m_active = false;
        return false;
    }

    return true;
}

void CompletionMenu::Render(void)
{
    if (!FilterCompletions())
        return;

    TextEditor &editor = *m_parentWindow->GetTextEditor();
    Preferences &preferences = m_application->GetPreferences();

    TextEditor::Coordinates cursor_pos;
    editor.GetCursorPosition(cursor_pos.mLine, cursor_pos.mColumn);

    const ImVec2 windowPos = ImGui::GetWindowPos();
    const ImVec2 windowSize = ImGui::GetWindowSize();

    auto len = editor.TextDistanceToLineStart(cursor_pos);
    float offsetX = windowPos.x - editor.GetScrollX() + len;
    float offsetY = windowPos.y - editor.GetScrollY() + (cursor_pos.mLine + 1) * preferences.GetGlobalSettings().editorFontSize;

    if (offsetY < windowPos.y || offsetX < windowPos.x)
        return;

    offsetX = ImClamp(offsetX, { 0 }, windowPos.x + windowSize.x - 200);

    ImGui::SetNextWindowPos(ImVec2(offsetX, offsetY));
    ImGui::SetNextWindowSizeConstraints(ImVec2(200, 0), ImVec2(FLT_MAX, 200));

    ImGui::SetNextWindowViewport(ImGui::GetWindowViewport()->ID);

	ImGuiStyle &style = ImGui::GetStyle();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.FramePadding);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, style.Colors[ImGuiCol_MenuBarBg]);
    if (ImGui::Begin("CompletionMenu", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_Tooltip
	))
    {
        if (ImGui::IsKeyPressed(ImGuiKey_Tab, false))
        {
            editor.SetReadOnlyEnabled(true);

            ImGui::SetWindowFocus();
            ImGui::NavMoveRequestSubmit(ImGuiDir_None, ImGuiDir_None, ImGuiNavMoveFlags_IsTabbing | ImGuiNavMoveFlags_FocusApi, ImGuiScrollFlags_KeepVisibleEdgeX | ImGuiScrollFlags_KeepVisibleEdgeY);
            if (!m_active)
            {
                GImGui->NavTabbingCounter = 1;
                m_active = true;
            }
        }
        
        for (const auto& completion : m_filteredCompletions)
        {
            if (ImGui::Selectable(completion.first.c_str()))
            {
				editor.ReplacePaste(completion.first.c_str());
                editor.SetReadOnlyEnabled(true);
                ImGui::SetNextWindowFocus();
            }
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
			switch (completion.second)
			{
			case COMPLETION_TYPE_TYPE: ImGui::TextUnformatted("type"); break;
			case COMPLETION_TYPE_FUNCTION: ImGui::TextUnformatted("function"); break;
			case COMPLETION_TYPE_VARIABLE: ImGui::TextUnformatted("variable"); break;
			case COMPLETION_TYPE_NAMESPACE: ImGui::TextUnformatted("namespace"); break;
			}
			ImGui::PopStyleColor();
        }
    }
    ImGui::End();
    ImGui::PopStyleColor();
	ImGui::PopStyleVar();

    if (ImGui::IsKeyPressed(ImGuiKey_Escape, false))
    {
        m_active = false;
        ImGui::SetNextWindowFocus();
    }
}
}

