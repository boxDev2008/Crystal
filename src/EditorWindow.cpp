#include "EditorWindow.h"
#include "Application.h"
#include "Resources.h"
#include "Utils.h"

#include "math/Vector2.h"

#include <fstream>
#include <sstream>
#include <iostream>

namespace Crystal
{

using namespace Math;

std::string ConvertTabsToSpaces(const std::string &input, int tabWidth)
{
    std::string result;
    int currentColumn = 0;

    for (char c : input)
	{
        if (c == '\t')
		{
            int spacesToNextTabStop = tabWidth - (currentColumn % tabWidth);
            result.append(spacesToNextTabStop, ' ');
            currentColumn += spacesToNextTabStop;
        }
		else
		{
            result += c;
            currentColumn++;
        }
    }

    return result;
}

/*const TextEditor::Palette &GetTokyoNightPalette()
{
	const static TextEditor::Palette p = { {
			0xFFFAC4C7,	// Default
			0xFFFF8BBC,	// Keyword	
			0xFFC7DB91,	// Number
			0xFF49D5A4,	// String
			0xFF49D5A4, // Char literal
			0xFFE3AD84, // Punctuation
			0xFFFF8BBC,	// Preprocessor
			0xFFFAC4C7, // Identifier
			0xFFFF948F, // Known identifier
			0xFFFF948F, // Preproc identifier
			0xFF735457, // Comment (single line)
			0xFF735457, // Comment (multi line)
			0xFF271a1b, // Background
			0xffe0e0e0, // Cursor
			0xFF422c2d, // Selection
			0x800020ff, // ErrorMarker
			0x40f08000, // Breakpoint
			0xFF57373a, // Line number
			0x40000000, // Current line fill
			0x40000000, // Current line fill (inactive)
			0x0, // Current line edge
		} };
	return p;
}*/

EditorWindow::EditorWindow(const std::filesystem::path &filePath)
{
	SetFilePath(filePath);

	/*static const char* ppnames[] = { "NULL", "PM_REMOVE",
		"ZeroMemory", "DXGI_SWAP_EFFECT_DISCARD", "D3D_FEATURE_LEVEL", "D3D_DRIVER_TYPE_HARDWARE", "WINAPI","D3D11_SDK_VERSION", "assert" };

	static const char* ppvalues[] = { 
		"#define NULL ((void*)0)", 
		"#define PM_REMOVE (0x0001)",
		"Microsoft's own memory zapper function\n(which is a macro actually)\nvoid ZeroMemory(\n\t[in] PVOID  Destination,\n\t[in] SIZE_T Length\n); ", 
		"enum DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_DISCARD = 0", 
		"enum D3D_FEATURE_LEVEL", 
		"enum D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE  = ( D3D_DRIVER_TYPE_UNKNOWN + 1 )",
		"#define WINAPI __stdcall",
		"#define D3D11_SDK_VERSION (7)",
		" #define assert(expression) (void)(                                                  \n"
        "    (!!(expression)) ||                                                              \n"
        "    (_wassert(_CRT_WIDE(#expression), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0) \n"
        " )"
		};

	for (int i = 0; i < sizeof(ppnames) / sizeof(ppnames[0]); ++i)
	{
		TextEditor::Identifier id;
		id.mDeclaration = ppvalues[i];
		lang.mPreprocIdentifiers.insert(std::make_pair(std::string(ppnames[i]), id));
	}*/

	m_editor.SetShowWhitespacesEnabled(false);
	m_editor.SetShowLineNumbersEnabled(false);
	m_editor.SetLineSpacing(1.1f);

    std::ifstream filestream(filePath);
    
    if (filestream.is_open())
	{
		std::ostringstream ss;
		ss << filestream.rdbuf();
        m_editor.SetText(ss.str());
		filestream.close();
    }
}

void EditorWindow::OnWindowAdded(void)
{
	m_editor.SetPalette(m_application->GetPreferences().GetGlobalSettings().GetEditorPalette());

	BuildErrorHandler &errorHandler = m_application->GetBuildErrorHandler();

	if (!errorHandler.HasAppliedErrorMarkers())
		return;

	std::vector<BuildErrorHandler::BuildError> filteredErrors = errorHandler.FilterErrorsByPath(m_filePath);

	if (filteredErrors.empty())
		return;

	TextEditor::ErrorMarkers markers;
	for (const auto &error : filteredErrors)
		markers.insert(std::make_pair(error.m_lineNumber - 1, error.m_errorMessage));
	m_editor.SetErrorMarkers(markers);
}

/*void EditorWindow::GetCurrentWord(std::string &word, int& start, int& end)
{
    std::string line = m_editor.GetCurrentLineText();
    const int cursorPosition = m_editor.GetCursorPosition().mColumn;

	line = ConvertTabsToSpaces(line, m_editor.GetTabSize());

    if (line.empty())
	{
        word.clear();
        start = end = cursorPosition;
        return;
    }

    // Check if the cursor is at the beginning of a word
    if (cursorPosition < line.size() && !Utils::isDelimiter(line[cursorPosition]))
    {
        if (cursorPosition == 0 || Utils::isDelimiter(line[cursorPosition - 1]))
        {
            // The cursor is at the beginning of a word
            word.clear();
            start = end = cursorPosition;
            return;
        }
    }

    start = cursorPosition;
    while (start > 0 && !Utils::isDelimiter(line[start - 1]))
        --start;

    end = cursorPosition;
    while (end < line.size() && !Utils::isDelimiter(line[end]))
        ++end;

    if (start < end && start >= 0 && end <= line.size())
        word = line.substr(start, end - start);
    else
        word.clear();
}*/

void EditorWindow::RenderWindow(void)
{
	std::string title = m_filePath.filename().string() + "##" + m_filePath.string();
	const char *c_title = title.c_str();

	//int start, end;
	//ImVec2 windowPosition;

	ImGui::SetNextWindowDockID(m_application->GetLayoutHandler().GetMainDockID(), ImGuiCond_Appearing);
	if (ImGui::Begin(c_title, &m_opened))
	{
		//SuggestionHandler &suggestionHandler = m_application->GetSuggestionHandler();
		Preferences &preferences = m_application->GetPreferences();
		WindowManager &wm = m_application->GetWindowManager();

		Preferences::EditorSettings &settings = preferences.GetEditorSettings();
		m_editor.SetTabSize(settings.tabSize);
		m_editor.SetLineSpacing(settings.lineSpacing);
		m_editor.SetShowLineNumbersEnabled(settings.showLineNumbers);
		m_editor.SetShowWhitespacesEnabled(settings.showWhitespaces);
		m_editor.SetShortTabsEnabled(settings.shortTabs);
		m_editor.SetAutoIndentEnabled(settings.autoIndent);

		//m_findReplaceHandler.Render(m_editor);

		/*std::string word;
		GetCurrentWord(word, start, end);
		suggestionHandler.FilterSuggestions(word);

		static bool focusSuggestions = false;
		const bool hasSuggestions = suggestionHandler.HasSuggestions();

		if (ImGui::IsWindowFocused())
			ImGui::SetNextWindowFocus();

		bool isWindowFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);

		if (isWindowFocused)
		{
			if (wm.GetLastWindow() != this)
				wm.SetLastWindow(this);
			
			if (hasSuggestions && ImGui::IsKeyPressed(ImGuiKey_Tab))
			{
				m_editor.SetReadOnly(true);
				focusSuggestions = true;
			}
		}

		m_editor.Render(c_title);

		if (hasSuggestions)
		{
			m_editor.SetReadOnly(false);

			if (wm.GetLastWindow() == this)
			{
				if (m_editor.IsFocused())
					ImGui::OpenPopup("##Suggestions");
				suggestionHandler.RenderSuggestions(m_editor, word, ImGui::GetWindowPos(), focusSuggestions, start, end);
			}
		}*/

		if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && wm.GetLastWindow() != this)
			wm.SetLastWindow(this);

		m_editor.Render(c_title);
	}
	ImGui::End();
}

void EditorWindow::SetFilePath(const std::filesystem::path &path)
{
	m_filePath = path;
	std::filesystem::path extension = m_filePath.extension();
	if (extension == ".cpp" || extension == ".hpp" || extension == ".h")
		m_editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Cpp());
	else if (extension == ".c")
		m_editor.SetLanguageDefinition(TextEditor::LanguageDefinition::C());
	else if (extension == ".cs")
		m_editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Cs());
	else if (extension == ".hlsl")
		m_editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Hlsl());
	else if (extension == ".glsl" || extension == ".shader" || extension == ".vert" || extension == ".frag")
		m_editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Glsl());
	else if (extension == ".py")
		m_editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Python());
	else if (extension == ".lua")
		m_editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
	else if (extension == ".rs")
		m_editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Rust());
	else if (extension == ".js" || extension == ".ts")
		m_editor.SetLanguageDefinition(TextEditor::LanguageDefinition::JavaScript());
	else if (extension == ".json")
		m_editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Json());
	else if (extension == ".ini")
		m_editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Ini());
	else
		m_editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Text());
}

void EditorWindow::SaveToFile(void)
{
	std::ofstream file(m_filePath);
	if (file.is_open())
	{
		file << m_editor.GetText();
		file.close();
	}
}

void EditorWindow::FindReplaceHandler::FindNext(TextEditor &editor, const std::string &word)
{
		
}

void EditorWindow::FindReplaceHandler::FindPrevious(TextEditor &editor, const std::string &word)
{

}

void EditorWindow::FindReplaceHandler::Replace(TextEditor &editor, const std::string &word, const std::string &replacement)
{

}

void EditorWindow::FindReplaceHandler::ReplaceAll(TextEditor &editor, const std::string &word, const std::string &replacement)
{

}

void EditorWindow::FindReplaceHandler::Render(TextEditor &editor)
{
	ImGuiStyle &style = ImGui::GetStyle();

	ImGui::BeginChild("##FindReplaceChild", ImVec2(0, 200), true);

	static char inputFindText[1024] = "";
	static char inputReplaceText[1024] = "";
	static bool matchCase = false;
	static bool matchWholeWord = false;

	ImGui::InputText("##Find", inputFindText, IM_ARRAYSIZE(inputFindText));
	if (!*inputFindText)
	{
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + style.WindowPadding.x * 0.5f + 1);
		float cursorPosY = ImGui::GetCursorPosY();
		ImGui::SetCursorPosY(cursorPosY - ImGui::GetFontSize() - style.WindowPadding.y * 0.5f);
		ImGui::TextDisabled("Find");
		ImGui::SetCursorPosY(cursorPosY);
	}
	ImGui::InputText("##Replace", inputReplaceText, IM_ARRAYSIZE(inputReplaceText));
	if (!*inputReplaceText)
	{
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + style.WindowPadding.x * 0.5f + 1);
		float cursorPosY = ImGui::GetCursorPosY();
		ImGui::SetCursorPosY(cursorPosY - ImGui::GetFontSize() - style.WindowPadding.y * 0.5f);
		ImGui::TextDisabled("Replace");
		ImGui::SetCursorPosY(cursorPosY);
	}
	ImGui::Checkbox("Match case", &matchCase);
	ImGui::SameLine();
	ImGui::Checkbox("Match whole word", &matchWholeWord);

	// Buttons for Find Next, Replace, and Replace All
	if (ImGui::Button("Find Next"))
	{
		FindNext(editor, inputFindText);
	}

	ImGui::SameLine();

	if (ImGui::Button("Find Previous"))
	{
		FindPrevious(editor, inputFindText);
	}

	ImGui::SameLine();
	if (ImGui::Button("Replace"))
	{
		Replace(editor, inputFindText, inputReplaceText);
	}

	ImGui::SameLine();
	if (ImGui::Button("Replace All"))
	{
		ReplaceAll(editor, inputFindText, inputReplaceText);
	}
	ImGui::EndChild();
}

}