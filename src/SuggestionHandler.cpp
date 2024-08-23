#include "SuggestionHandler.h"
#include "Resources.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <stdlib.h>

namespace Crystal
{

void SuggestionHandler::FilterSuggestions(const std::string& input)
{
    m_suggestionData.m_filteredSuggestions.clear();

    if (input.empty()) return;

    for (const auto &suggestion : m_suggestionData.m_initialKeywords)
        if (suggestion.find(input) != std::string::npos && suggestion != input)
            m_suggestionData.m_filteredSuggestions.push_back({Suggestion::Type::Keyword, suggestion });

    for (const auto &suggestion : m_suggestionData.m_initialMethods)
        if (suggestion.find(input) != std::string::npos && suggestion != input)
            m_suggestionData.m_filteredSuggestions.push_back({Suggestion::Type::Method, suggestion });

    for (const auto &suggestion : m_suggestionData.m_initialClasses)
        if (suggestion.find(input) != std::string::npos && suggestion != input)
            m_suggestionData.m_filteredSuggestions.push_back({Suggestion::Type::Class, suggestion });

    for (const auto &suggestion : m_suggestionData.m_additionalSuggestions)
        if (suggestion.word.find(input) != std::string::npos && suggestion.word != input)
            m_suggestionData.m_filteredSuggestions.push_back(suggestion);
}

void SuggestionHandler::InsertSuggestion(TextEditor &editor, const std::string &initialWord, const std::string &suggestion, int wordStart, int wordEnd)
{
    /*TextEditor::EditorState beforeState = editor.GetState();

    editor.SetCursorPosition(TextEditor::Coordinates(beforeState.mCursorPosition.mLine, wordStart));
    editor.DeleteRange(TextEditor::Coordinates(beforeState.mCursorPosition.mLine, wordStart), TextEditor::Coordinates(beforeState.mCursorPosition.mLine, wordEnd));
    editor.InsertText(suggestion.c_str());

    TextEditor::EditorState afterState;
    afterState.mCursorPosition = editor.GetCursorPosition();

    TextEditor::UndoRecord undoRecord(
        suggestion,
        TextEditor::Coordinates(beforeState.mCursorPosition.mLine, wordStart),
        TextEditor::Coordinates(beforeState.mCursorPosition.mLine, wordStart + suggestion.size()),

        initialWord,
        TextEditor::Coordinates(beforeState.mCursorPosition.mLine, wordStart),
        TextEditor::Coordinates(beforeState.mCursorPosition.mLine, wordEnd),

        beforeState,
        afterState
    );

    editor.AddUndo(undoRecord);*/
}

void SuggestionHandler::RenderSuggestions(TextEditor &editor, const std::string &initialWord, const ImVec2 &windowPosition, bool &focus, int wordStart, int wordEnd)
{
	/*auto pos = editor.GetCursorPosition();
	auto len = editor.TextDistanceToLineStart(pos);

	float offsetX = windowPosition.x - editor.GetScrollX() + len + 56;
	float offsetY = windowPosition.y - editor.GetScrollY() + pos.mLine * 19 + 70;

	ImGui::SetNextWindowSizeConstraints(ImVec2(100.0f, 32.0f), ImVec2(400.0f, 160.0f));
	ImGui::SetNextWindowPos(ImVec2(offsetX, offsetY));

	ImGuiWindow *previousWindow = ImGui::GetCurrentWindow();
	if (ImGui::BeginPopup("##Suggestions", ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
	{
		ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
		ImGui::SetScrollX(0);

		if (focus)
		{
			ImGui::SetWindowFocus();
			ImGui::NavMoveRequestSubmit(ImGuiDir_None, ImGuiDir_None, ImGuiNavMoveFlags_IsTabbing | ImGuiNavMoveFlags_FocusApi, ImGuiScrollFlags_KeepVisibleEdgeX | ImGuiScrollFlags_KeepVisibleEdgeY);
			//ImGui::NavMoveRequestSubmit(ImGuiDir_None, ImGuiDir_None, ImGuiNavMoveFlags_IsTabbing, 0);
			GImGui->NavTabbingCounter = 1;
			focus = false;
		}
		else
			GImGui->NavTabbingCounter = 0;

		for (const auto &suggestion : m_suggestionData.m_filteredSuggestions)
		{
			switch (suggestion.type)
			{
			case Suggestion::Type::Keyword:
				ImGui::Image((ImTextureID)Resources::GetTextureByName("icon-keyword").id, ImVec2(16.0f, 16.0f));
				break;
			case Suggestion::Type::Method:
				ImGui::Image((ImTextureID)Resources::GetTextureByName("icon-method").id, ImVec2(16.0f, 16.0f));
				break;
			case Suggestion::Type::Class:
				ImGui::Image((ImTextureID)Resources::GetTextureByName("icon-class").id, ImVec2(16.0f, 16.0f));
				break;
			case Suggestion::Type::Variable:
				ImGui::Image((ImTextureID)Resources::GetTextureByName("icon-variable").id, ImVec2(16.0f, 16.0f));
				break;
			}

			ImGui::SameLine();
			if (ImGui::Selectable(suggestion.word.c_str()))
			{
				InsertSuggestion(editor, initialWord, suggestion.word, wordStart, wordEnd);
				m_suggestionData.m_filteredSuggestions.clear();
				break;
			}
		}
		ImGui::EndPopup();
	}*/
}

}