#include "EditorWindow.h"
#include "Application.h"
#include "Resources.h"
#include "Utils.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <stack>
#include <chrono>

#include "imgui_internal.h"

#include <tree_sitter/tree_sitter_cpp.h>

namespace Crystal
{

static TSPoint ToTSPoint(const TextEditor::Coordinates& coords)
{
    return TSPoint{ (uint32_t)coords.mLine, (uint32_t)coords.mColumn };
}

static TSInputEdit ConvertUndoToInputEdit(const TextEditor::UndoOperation& op, const TextEditor& editor, const std::vector<std::string> &lines)
{
    TSPoint startPoint = ToTSPoint(op.mStart);
    TSPoint oldEndPoint = ToTSPoint(op.mEnd);
    TSPoint newEndPoint = startPoint;

    /*std::cout << "Undo Operation: " << startPoint.row << " " << startPoint.column << " " << oldEndPoint.row << " " << oldEndPoint.column << std::endl;
    std::cout << "Text size: " << op.mText.size() << std::endl;
    std::cout << "Type: " << (op.mType == TextEditor::UndoOperationType::Add ? "Add" : "Delete") << std::endl;
    std::cout << op.mText << std::endl;*/

    uint32_t startIndex = editor.GetGlobalIndexFromPosition(op.mStart);
    uint32_t oldEndIndex = startIndex;
    uint32_t newEndIndex = startIndex;

    //std::cout << "Start index: " << startIndex << std::endl;

    if (op.mType == TextEditor::UndoOperationType::Delete)
        oldEndIndex += op.mText.size();
    else if (op.mType == TextEditor::UndoOperationType::Add)
    {
        newEndIndex += op.mText.size();
        newEndPoint.column += (uint32_t)op.mText.size();
    }

    return TSInputEdit{
        startIndex,
        oldEndIndex,
        newEndIndex,
        startPoint,
        oldEndPoint,
        newEndPoint
    };
}

void EditorWindow::AddMultilineErrorMarker(TextEditor& editor, int startLine, int startColumn, int endLine, int endColumn, const std::string& fullMessage)
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

void EditorWindow::ExtractCompletions(TSNode node, const std::string& source)
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
        AddMultilineErrorMarker(m_editor, ts_node_start_point(node).row + 1, ts_node_start_point(node).column, ts_node_end_point(node).row + 1, ts_node_end_point(node).column, "Syntax Error");
        //editor.AddErrorMarker(ts_node_start_point(node).row + 1, ts_node_start_point(node).column, ts_node_end_point(node).column, "Missing Symbol");
    }
    else if (ts_node_is_missing(node))
    {
        AddMultilineErrorMarker(m_editor, ts_node_start_point(node).row + 1, ts_node_start_point(node).column, ts_node_end_point(node).row + 1, ts_node_end_point(node).column, "Missing Symbol");
        //editor.AddErrorMarker(ts_node_start_point(node).row + 1, ts_node_start_point(node).column, ts_node_end_point(node).column, "Missing Symbol");
    }
    else if (type == "identifier")
    {
        TSNode parent = ts_node_parent(node);
        const char *parentType = ts_node_type(parent);
        if (!strcmp(parentType, "function_declarator") || !strcmp(parentType, "qualified_identifier")) // function names
            m_completionMenu.AddCompletion(nodeText, COMPLETION_TYPE_FUNCTION);
        else m_completionMenu.AddCompletion(nodeText, COMPLETION_TYPE_VARIABLE);
    }
    else if (type == "field_identifier")
        m_completionMenu.AddCompletion(nodeText, COMPLETION_TYPE_VARIABLE);
    else if (type == "type_identifier")
    {
        TSNode parent = ts_node_parent(node);
        const char *parentType = ts_node_type(parent);
        if (!strcmp(parentType, "struct_specifier") || !strcmp(parentType, "class_specifier") || !strcmp(parentType, "class_specifier"))
            m_completionMenu.AddCompletion(nodeText, COMPLETION_TYPE_TYPE);
    }
    else if (type == "primitive_type")
        m_completionMenu.AddCompletion(nodeText, COMPLETION_TYPE_TYPE);
    else if (type == "namespace_identifier")
        m_completionMenu.AddCompletion(nodeText, COMPLETION_TYPE_NAMESPACE);

    uint32_t count = ts_node_child_count(node);
    for (uint32_t i = 0; i < count; ++i)
        ExtractCompletions(ts_node_child(node, i), source);
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

    std::string &text = m_editor.GetText();

    m_parser = ts_parser_new();
    ts_parser_set_language(m_parser, tree_sitter_cpp());
    m_tree = ts_parser_parse_string(m_parser, nullptr, text.c_str(), text.size());
    ExtractCompletions(ts_tree_root_node(m_tree), text);

    m_editor.SetRecordCallback([this](const TextEditor::UndoRecord &_record) {
        if (_record.mOperations.empty())
            return;

        TextEditor::UndoRecord record = _record;

        //auto start = std::chrono::high_resolution_clock::now();

        std::string &text = m_editor.GetText();
        std::vector<std::string> &lines = m_editor.GetTextLines();

        if (record.mTSFlags & TextEditor::TSFlag::ReverseOperation)
            std::reverse(record.mOperations.begin(), record.mOperations.end());

        for (int i = (int)record.mOperations.size() - 1; i >= 0; --i)
        //for (int i = 0; i < (int)record.mOperations.size(); ++i)
        {
            TSInputEdit edit = ConvertUndoToInputEdit(record.mOperations[i], m_editor, lines);
            ts_tree_edit(m_tree, &edit);
        }

        TSTree *newTree = ts_parser_parse_string(m_parser, m_tree, text.c_str(), text.size());
        ts_tree_delete(m_tree);
        m_tree = newTree;

        m_editor.ClearErrorMarkers();
        m_completionMenu.ClearCompletions();
        ExtractCompletions(ts_tree_root_node(m_tree), text);

        //auto end = std::chrono::high_resolution_clock::now();
        //std::cout << "Time taken to parse: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

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

		m_findReplaceHandler.Render(m_editor);

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
		m_editor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Cpp);
	else if (extension == ".c")
		m_editor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::C);
	else if (extension == ".cs")
		m_editor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Cs);
	else if (extension == ".hlsl")
		m_editor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Hlsl);
	else if (extension == ".glsl" || extension == ".shader" || extension == ".vert" || extension == ".frag")
		m_editor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Glsl);
	else if (extension == ".py")
		m_editor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Python);
	else if (extension == ".lua")
		m_editor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Lua);
	/*else if (extension == ".rs")
		m_editor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Rust);
	else if (extension == ".js" || extension == ".ts")
		m_editor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Javascript);*/
	else if (extension == ".json")
		m_editor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Json);
	/*else if (extension == ".ini")
		m_editor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Ini);
	else if (extension == ".html")
		m_editor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Html);
	else if (extension == ".css")
		m_editor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Css);
	else if (extension == ".gml")
		m_editor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Gml);*/
	else
		m_editor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::None);
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
    if (editor.AnyCursorHasSelection())
	    editor.ReplacePaste(replacement.c_str());
}

void EditorWindow::FindReplaceHandler::ReplaceAll(TextEditor &editor, const std::string &word, const std::string &replacement)
{
	editor.SelectAllOccurrencesOf(word.c_str(), word.length(), m_caseSensitive);
    if (editor.AnyCursorHasSelection())
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