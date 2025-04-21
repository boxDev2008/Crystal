#include "EditorWindow.h"
#include "Application.h"
#include "Resources.h"
#include "Utils.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include "imgui_internal.h"

#include <tree_sitter/tree_sitter_cpp.h>

namespace Crystal
{

void AddMultilineErrorMarker(TextEditor& editor, int startLine, int startColumn, int endLine, int endColumn, const std::string& fullMessage)
{
    std::istringstream stream(fullMessage);
    std::string lineMessage;
    int currentLine = startLine;

    while (std::getline(stream, lineMessage))
	{
        int thisStartColumn = (currentLine == startLine) ? startColumn : 0;
        int thisEndColumn = (currentLine == endLine) ? endColumn : lineMessage.size();

        editor.AddErrorMarker(currentLine, thisStartColumn, thisEndColumn, lineMessage);
        currentLine++;
    }
}

void ExtractCompletions(TSNode node, const std::string& source, TextEditor &editor, CompletionMenu &menu)
{
    if (ts_node_is_null(node)) return;

    std::string type = ts_node_type(node);
    std::string nodeText = source.substr(ts_node_start_byte(node), ts_node_end_byte(node) - ts_node_start_byte(node));
    
    /*std::cout << "Node Type: " << type << std::endl;
    std::cout << "Node Text: " << nodeText << std::endl;
    std::cout << "Start Byte: " << ts_node_start_byte(node) << ", End Byte: " << ts_node_end_byte(node) << std::endl;
    std::cout << "Child Count: " << ts_node_child_count(node) << std::endl;
    std::cout << "-------------------" << std::endl;*/

    if (ts_node_is_error(node))
    {
        //std::cout << "Error: " << nodeText << '\n';
        AddMultilineErrorMarker(editor, ts_node_start_point(node).row + 1, ts_node_start_point(node).column, ts_node_end_point(node).row + 1, ts_node_end_point(node).column, "Syntax Error");
        //editor.AddErrorMarker(ts_node_start_point(node).row + 1, ts_node_start_point(node).column, ts_node_end_point(node).column, "Missing Symbol");
    }
    else if (ts_node_is_missing(node))
    {
        AddMultilineErrorMarker(editor, ts_node_start_point(node).row + 1, ts_node_start_point(node).column, ts_node_end_point(node).row + 1, ts_node_end_point(node).column, "Missing Symbol");
        //editor.AddErrorMarker(ts_node_start_point(node).row + 1, ts_node_start_point(node).column, ts_node_end_point(node).column, "Missing Symbol");
    }
    else if (type == "identifier")
    {
        TSNode parent = ts_node_parent(node);
        const char *parentType = ts_node_type(parent);
        if (!strcmp(parentType, "function_declarator") || !strcmp(parentType, "qualified_identifier")) // function names
		{
			menu.AddCompletion(nodeText, COMPLETION_TYPE_FUNCTION);
			//parameter_list
		}
        else // if (!strcmp(parentType, "init_declarator") || !strcmp(parentType, "declaration")) // variable names
			menu.AddCompletion(nodeText, COMPLETION_TYPE_VARIABLE);
    }
    else if (type == "field_identifier") // field variable names
    {
        menu.AddCompletion(nodeText, COMPLETION_TYPE_VARIABLE);
    }
    else if (type == "type_identifier")
    {
        TSNode parent = ts_node_parent(node);
        const char *parentType = ts_node_type(parent);
        if (!strcmp(parentType, "struct_specifier") || !strcmp(parentType, "class_specifier")) // struct/class names
			menu.AddCompletion(nodeText, COMPLETION_TYPE_TYPE);
    }
    else if (type == "primitive_type")
    {
        menu.AddCompletion(nodeText, COMPLETION_TYPE_TYPE);            
    }
	else if (type == "namespace_identifier")
	{
		menu.AddCompletion(nodeText, COMPLETION_TYPE_NAMESPACE);
	}

    uint32_t count = ts_node_child_count(node);
    for (uint32_t i = 0; i < count; ++i)
        ExtractCompletions(ts_node_child(node, i), source, editor, menu);
}

EditorWindow::EditorWindow(const std::filesystem::path &filePath)
{
	SetFilePath(filePath);
}

EditorWindow::~EditorWindow(void)
{
    if (m_tree)
        ts_tree_delete(m_tree);

    if (m_parser)
        ts_parser_delete(m_parser);
	
	WindowManager &wm = m_application->GetWindowManager();
	if (wm.GetLastEditorWindow() == this)
		wm.SetLastEditorWindow(nullptr);
}

void EditorWindow::OnWindowAdded(void)
{
	m_editor.SetPalette(m_application->GetPreferences().GetGlobalSettings().GetEditorPalette());

    std::ifstream filestream(m_filePath);
    
    if (filestream.is_open())
	{
		std::ostringstream ss;
		ss << filestream.rdbuf();
        m_editor.SetText(ss.str());
		m_editor.SetTextChanged(false);
		filestream.close();
    }

    const char *languageName = m_editor.GetLanguageDefinitionName();
    if (strcmp(languageName, "C++") && strcmp(languageName, "C"))
        return;
	
	m_completionMenu = CompletionMenu(this, m_application);

    m_parser = ts_parser_new();
    ts_parser_set_language(m_parser, tree_sitter_cpp());
    m_tree = ts_parser_parse_string(m_parser, nullptr, m_editor.GetText().c_str(), m_editor.GetText().size());
    ExtractCompletions(ts_tree_root_node(m_tree), m_editor.GetText(), m_editor, m_completionMenu);

    m_editor.SetRecordCallback([this](const TextEditor::UndoRecord &record) {
        if (record.mOperations.empty())
            return;

        //TSNode rootNode = ts_tree_root_node(m_tree);

        for (const auto& op : record.mOperations)
        {
			TSInputEdit inputEdit = TreeEditForUndoOperation(op, m_editor.GetTextLines());
			ts_tree_edit(m_tree, &inputEdit);
			TSTree *newTree = ts_parser_parse_string(m_parser, m_tree, m_editor.GetText().c_str(), m_editor.GetText().size());
			ts_tree_delete(m_tree);
			m_tree = newTree;
        }

        m_editor.ClearErrorMarkers();
        m_completionMenu.ClearCompletions();
        ExtractCompletions(ts_tree_root_node(m_tree), m_editor.GetText(), m_editor, m_completionMenu);
    });
}

void EditorWindow::GetCurrentWord(std::string &word, int& start, int& end)
{
	TextEditor::Coordinates cursorPosition;
	m_editor.GetCursorPosition(cursorPosition.mLine, cursorPosition.mColumn);
    std::string line = m_editor.GetTextLines()[cursorPosition.mLine];

	const int32_t column = m_editor.GetCharacterIndexL(cursorPosition);

    if (line.empty())
	{
        word.clear();
        start = end = column;
        return;
    }

    // Check if the cursor is at the beginning of a word
    if (column < line.size() && !Utils::isDelimiter(line[column]))
    {
        if (column == 0 || Utils::isDelimiter(line[column - 1]))
        {
            // The cursor is at the beginning of a word
            word.clear();
            start = end = column;
            return;
        }
    }

    start = column;
    while (start > 0 && !Utils::isDelimiter(line[start - 1]))
        --start;

    end = column;
    while (end < line.size() && !Utils::isDelimiter(line[end]))
        ++end;

    if (start < end && start >= 0 && end <= line.size())
        word = line.substr(start, end - start);
    else
        word.clear();
}

void EditorWindow::RenderWindow(void)
{
    ImGui::SetNextWindowDockID(m_application->GetLayoutHandler().GetMainDockID(), ImGuiCond_Appearing);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });

    ImGuiWindowFlags flags = m_editor.IsTextChanged() ? ImGuiWindowFlags_UnsavedDocument : 0;
    if (ImGui::Begin(m_titleBuffer, &m_opened, flags))
    {
        PlatformWindow &mainWindow = m_application->GetMainWindow();
        WindowManager &wm = m_application->GetWindowManager();
        Preferences &preferences = m_application->GetPreferences();
        Preferences::EditorSettings &settings = preferences.GetEditorSettings();

        m_editor.SetTabSize(settings.tabSize);
        m_editor.SetLineSpacing(settings.lineSpacing);
        m_editor.SetShowLineNumbersEnabled(settings.showLineNumbers);
        m_editor.SetShowWhitespacesEnabled(settings.showWhitespaces);
        m_editor.SetShortTabsEnabled(settings.shortTabs);
        m_editor.SetAutoIndentEnabled(settings.autoIndent);

        ImGui::BeginChild("##Editor");
        ImGui::SetWindowFontScale(preferences.GetGlobalSettings().editorFontSize / 24.0f / ImGui::GetIO().FontGlobalScale);

        const bool isFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
        
		if (!wm.GetLastEditorWindow())
			wm.SetLastEditorWindow(this);
		else if (wm.GetLastEditorWindow() != this && isFocused)
            wm.SetLastEditorWindow(this);

        if (m_tree && wm.GetLastEditorWindow() == this)
        {
            static std::string word;
            static int32_t start, end;
            GetCurrentWord(word, start, end);
            m_completionMenu.SetCurrentWord(word, start, end);
            m_completionMenu.Render();
        }

		//m_findReplaceHandler.Render(m_editor);

        m_editor.Render(m_titleBuffer);
        m_editor.SetReadOnlyEnabled(false);

        ImGui::EndChild();

        if (isFocused && wm.GetLastEditorWindow() != this)
            wm.SetLastEditorWindow(this);
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void EditorWindow::SetFilePath(const std::filesystem::path &path)
{
	m_filePath = path;
	sprintf(m_titleBuffer, "%s##%s", path.filename().string().c_str(), path.string().c_str());

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
	else if (extension == ".html")
		m_editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Html());
	else if (extension == ".css")
		m_editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Css());
	else if (extension == ".gml")
		m_editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Gml());
	else
		m_editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Text());
}

void EditorWindow::SaveToFile(void)
{
	std::ofstream file(m_filePath);
	if (file.is_open())
	{
		file << m_editor.GetText();
		m_editor.SetTextChanged(false);
		file.close();
	}
}

void EditorWindow::FindReplaceHandler::FindNext(TextEditor &editor, const std::string &word)
{
	editor.SelectNextOccurrenceOf(word.c_str(), word.length(), m_caseSensitive);
}

void EditorWindow::FindReplaceHandler::FindAll(TextEditor &editor, const std::string &word)
{
	editor.SelectAllOccurrencesOf(word.c_str(), word.length(), m_caseSensitive);
}

void EditorWindow::FindReplaceHandler::ReplaceNext(TextEditor &editor, const std::string &word, const std::string &replacement)
{
	editor.SelectNextOccurrenceOf(word.c_str(), word.length(), m_caseSensitive);
	editor.ReplacePaste(replacement.c_str());
}

void EditorWindow::FindReplaceHandler::ReplaceAll(TextEditor &editor, const std::string &word, const std::string &replacement)
{
	editor.SelectAllOccurrencesOf(word.c_str(), word.length(), m_caseSensitive);
	editor.ReplacePaste(replacement.c_str());
}

void EditorWindow::FindReplaceHandler::Render(TextEditor &editor)
{
	if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_F))
		m_active = !m_active;
	else if (ImGui::IsKeyPressed(ImGuiKey_Escape))
		m_active = false;

	if (!m_active)
		return;

	ImVec2 parentPos = ImGui::GetWindowPos();

	ImGuiStyle& style = ImGui::GetStyle();
	ImVec2 windowSize = ImVec2(600, 120); // Adjust height/width as needed
	ImVec2 windowPos = ImVec2(windowSize.x - style.FramePadding.x + parentPos.x,
							style.FramePadding.y + parentPos.y); // Top-right with 20px margin
	
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
	
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
								ImGuiWindowFlags_NoResize |
								ImGuiWindowFlags_NoMove |
								ImGuiWindowFlags_NoSavedSettings |
								ImGuiWindowFlags_NoDocking;
	
	if (ImGui::Begin("FindReplaceBar", nullptr, flags))
	{
		static char inputFindText[1024] = "";
		static char inputReplaceText[1024] = "";
	
		float itemWidth = ImGui::GetContentRegionAvail().x * 0.35f;
		
		ImGui::PushItemWidth(itemWidth);
		ImGui::InputTextWithHint("##Find", "Find", inputFindText, IM_ARRAYSIZE(inputFindText));
		ImGui::SameLine();
		ImGui::InputTextWithHint("##Replace", "Replace", inputReplaceText, IM_ARRAYSIZE(inputReplaceText));
		ImGui::PopItemWidth();
	
		ImGui::SameLine();
		ImGui::Checkbox("Match case", &m_caseSensitive);
		
		ImGui::NewLine();
	
		if (ImGui::Button("Find Next"))
			FindNext(editor, inputFindText);
		ImGui::SameLine();
	
		if (ImGui::Button("Find All"))
			FindAll(editor, inputFindText);
		ImGui::SameLine();
	
		if (ImGui::Button("Replace"))
			ReplaceNext(editor, inputFindText, inputReplaceText);
		ImGui::SameLine();
	
		if (ImGui::Button("Replace All"))
			ReplaceAll(editor, inputFindText, inputReplaceText);
	}
	ImGui::End();
}

}