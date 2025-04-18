#include "CompletionMenu.h"

#include "imgui_internal.h"

#include <vector>
#include <algorithm>

namespace Crystal
{

struct CompletionMenuData
{
    bool active = false;
    int32_t wordStart, wordEnd;

    std::string word;

    std::unordered_set<std::string> completions{};
    std::vector<std::string> filteredCompletions{};
};

static CompletionMenuData s_data;

void CompletionMenu::SetCurrentWord(const std::string &word, int32_t wordStart, int32_t wordEnd)
{
    s_data.word = word;

    if (word.empty())
        return;

    s_data.wordStart = wordStart;
    s_data.wordEnd = wordEnd;
}

void CompletionMenu::AddCompletion(const std::string &completion)
{
    s_data.completions.insert(completion);
}

void CompletionMenu::ClearCompletions(void)
{
    s_data.completions.clear();
}

bool CompletionMenu::FilterCompletions(void)
{
    if (s_data.word.empty() || s_data.completions.empty())
    {
        s_data.active = false;
        return false;
    }
    
    s_data.filteredCompletions.clear();

    for (const auto& completion : s_data.completions)
    {
        std::string completionLower = completion;
        std::transform(completionLower.begin(), completionLower.end(), completionLower.begin(), ::tolower);
        std::string wordLower = s_data.word;
        std::transform(wordLower.begin(), wordLower.end(), wordLower.begin(), ::tolower);

        if (completion == s_data.word)
            continue;

        if (completionLower.find(wordLower) != std::string::npos)
            s_data.filteredCompletions.push_back(completion);
    }

    if (s_data.filteredCompletions.empty())
    {
        s_data.active = false;
        return false;
    }

    return true;
}

void CompletionMenu::RenderCompletionMenu(EditorWindow *parentWindow, Application *application)
{
    if (!FilterCompletions())
        return;

    TextEditor &editor = *parentWindow->GetTextEditor();
    Preferences &preferences = application->GetPreferences();

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
    ImGui::SetNextWindowSizeConstraints(ImVec2(200, 0), ImVec2(300, 200));

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::GetStyle().Colors[ImGuiCol_MenuBarBg]);
    if (ImGui::Begin("CompletionMenu", nullptr, 
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing))
    {
        ImGui::SetWindowPos(ImVec2(offsetX, offsetY));

        if (ImGui::IsKeyPressed(ImGuiKey_Tab, false))
        {
            editor.SetReadOnlyEnabled(true);

            ImGui::SetWindowFocus();
            ImGui::NavMoveRequestSubmit(ImGuiDir_None, ImGuiDir_None, ImGuiNavMoveFlags_IsTabbing | ImGuiNavMoveFlags_FocusApi, ImGuiScrollFlags_KeepVisibleEdgeX | ImGuiScrollFlags_KeepVisibleEdgeY);
            if (!s_data.active)
            {
                GImGui->NavTabbingCounter = 1;
                s_data.active = true;
            }
        }
        
        for (const auto& completion : s_data.filteredCompletions)
        {
            if (ImGui::Selectable(completion.c_str()))
            {
                TextEditor::UndoRecord u;
                u.mBefore = editor.GetState();

                if (!s_data.word.empty()) {
                    u.mOperations.push_back({
                        s_data.word,
                        TextEditor::Coordinates(u.mBefore.mCursors[0].mInteractiveStart.mLine, s_data.wordStart),
                        TextEditor::Coordinates(u.mBefore.mCursors[0].mInteractiveStart.mLine, s_data.wordEnd),
                        TextEditor::UndoOperationType::Delete
                    });
                }

                u.mOperations.push_back({
                    completion,
                    TextEditor::Coordinates(u.mBefore.mCursors[0].mInteractiveStart.mLine, s_data.wordStart),
                    TextEditor::Coordinates(u.mBefore.mCursors[0].mInteractiveStart.mLine, s_data.wordStart + completion.length()),
                    TextEditor::UndoOperationType::Add
                });

                editor.SetCursorPosition(TextEditor::Coordinates(u.mBefore.mCursors[0].mInteractiveStart.mLine, s_data.wordStart));
                if (!s_data.word.empty()) {
                    editor.DeleteRange(
                        TextEditor::Coordinates(u.mBefore.mCursors[0].mInteractiveStart.mLine, s_data.wordStart),
                        TextEditor::Coordinates(u.mBefore.mCursors[0].mInteractiveStart.mLine, s_data.wordEnd)
                    );
                }
                editor.InsertTextAtCursor(completion.c_str());

                u.mAfter = editor.GetState();

                editor.AddUndo(u);

                editor.SetReadOnlyEnabled(true);
                ImGui::SetNextWindowFocus();
            }
        }
    }
    ImGui::End();
    ImGui::PopStyleColor();

    if (ImGui::IsKeyPressed(ImGuiKey_Escape, false))
    {
        s_data.active = false;
        ImGui::SetNextWindowFocus();
    }
}
}

