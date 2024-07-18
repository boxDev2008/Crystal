#include "EditorWindow.h"
#include "Application.h"
#include "Resources.h"
#include "Utils.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include "imgui_internal.h"

namespace Crystal
{

struct Suggestion
{
	enum Type
	{
		Keyword,
		Method,
		Class
		//Variable
	};
	Type type;
	std::string word;
};

struct SuggestionData
{
	std::vector<std::string> possibleKeywords = {
		"int", "float", "void", "bool", "unsigned",
		"double", "char", "long", "short", "const",
		"static", "struct", "class", "enum", "namespace",
		"template", "typename", "size_t", "nullptr", "virtual",
		"volatile", "mutable", "friend", "operator", "extern",
		"inline", "typedef", "decltype", "auto", "register",
		"explicit", "private", "protected", "public", "using",
		"throw", "try", "catch", "if", "else",
		"switch", "case", "default", "while", "do",
		"for", "break", "continue", "return", "goto",
		"const_cast", "dynamic_cast", "reinterpret_cast", "static_cast",
		"alignas", "alignof", "char16_t", "char32_t", "constexpr",
		"final", "override", "transaction_safe", "thread_local",
		"asm", "sizeof", "typeof",
		"__declspec", "__attribute__", "__cdecl", "__stdcall",
		"__int8", "__int16", "__int32", "__int64", "__asm__",
		"_Alignas", "_Alignof", "_Atomic", "_Bool", "_Complex",
		"_Generic", "_Imaginary", "_Noreturn", "_Static_assert", "_Thread_local"
	};

	std::vector<std::string> possibleMethods = {
		"printf", "scanf", "malloc", "free", "calloc", "realloc",
		"strcpy", "strncpy", "strcat", "strncat", "strcmp", "strncmp",
		"strlen", "strchr", "strrchr", "strstr", "strtok", "strerror",
		"memcpy", "memmove", "memcmp", "memset",
		"fprintf", "fscanf", "fgets", "fputs", "fclose", "fopen", "freopen", "feof", "ferror", "fflush", "fputc", "fgetc", "rewind",
		"abs", "fabs", "sqrt", "pow", "ceil", "floor", "round", "rand", "srand",
		"time", "clock", "ctime", "localtime", "gmtime", "strftime", "difftime", "mktime",
		"sin", "cos", "tan", "asin", "acos", "atan", "atan2",
		"sinh", "cosh", "tanh", "asinh", "acosh", "atanh",
		"atoi", "atol", "atof", "itoa", "ltoa", "itoa_s", "itoa_l",
		"cout", "cin", "cerr", "endl", "getline", "stringstream", "string", "push_back", "pop_back", "clear", "map", "set",
		"unordered_map", "erase", "insert", "sort", "find", "max", "min"
	};

	std::vector<std::string> possibleClasses = {
		"std", "vector", 
		"int8_t", "uint8_t", "int16_t", "uint16_t", "int32_t", "uint32_t",
		"int64_t", "uint64_t", "intptr_t", "uintptr_t"
	};

	std::vector<Suggestion> filteredSuggestions;
};

static SuggestionData s_suggestionData;

void FilterSuggestions(const std::string& input)
{
    s_suggestionData.filteredSuggestions.clear();

    if (input.empty()) return;

    for (const auto &suggestion : s_suggestionData.possibleKeywords)
        if (suggestion.find(input) != std::string::npos && suggestion != input)
            s_suggestionData.filteredSuggestions.push_back({Suggestion::Type::Keyword, suggestion});

    for (const auto &suggestion : s_suggestionData.possibleMethods)
        if (suggestion.find(input) != std::string::npos && suggestion != input)
            s_suggestionData.filteredSuggestions.push_back({Suggestion::Type::Method, suggestion});

    for (const auto &suggestion : s_suggestionData.possibleClasses)
        if (suggestion.find(input) != std::string::npos && suggestion != input)
            s_suggestionData.filteredSuggestions.push_back({Suggestion::Type::Class, suggestion});
}

void InsertSuggestion(TextEditor& editor, const std::string &initialWord, const std::string& suggestion, int wordStart, int wordEnd)
{
    TextEditor::EditorState beforeState = editor.GetState();

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

    editor.AddUndo(undoRecord);
}

void RenderSuggestions(TextEditor &editor, const std::string &initialWord, const ImVec2 &windowPosition, bool &focus, int wordStart, int wordEnd)
{
	auto pos = editor.GetCursorPosition();
	auto len = editor.TextDistanceToLineStart(pos);

	ImGui::SetNextWindowSizeConstraints(ImVec2(64.0f, 32.0f), ImVec2(FLT_MAX, 200.0f));
	if (ImGui::BeginPopup("##Suggestions", ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
		))
	{
		ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());

		if (focus)
		{
			ImGui::SetWindowFocus();
			ImGui::NavMoveRequestSubmit(ImGuiDir_None, ImGuiDir_None, ImGuiNavMoveFlags_IsTabbing | ImGuiNavMoveFlags_FocusApi, ImGuiScrollFlags_KeepVisibleEdgeX | ImGuiScrollFlags_KeepVisibleEdgeY);
			GImGui->NavTabbingCounter = 1;
			focus = false;
		}
		else
			GImGui->NavTabbingCounter = 0;

		float offsetX = windowPosition.x + ImGui::GetCursorPosX() - editor.GetScrollX() + len + 32;
		float offsetY = windowPosition.y + ImGui::GetCursorPosY() - editor.GetScrollY() + pos.mLine * 20 + 64;

		ImGui::SetWindowPos(ImVec2(offsetX, offsetY));

		for (const auto &suggestion : s_suggestionData.filteredSuggestions)
		{
			switch (suggestion.type)
			{
			case Suggestion::Type::Keyword:
				ImGui::Image((ImTextureID)Resources::GetTextureByName("icon-keyword").descriptorSet, ImVec2(16.0f, 16.0f));
				break;
			case Suggestion::Type::Method:
				ImGui::Image((ImTextureID)Resources::GetTextureByName("icon-method").descriptorSet, ImVec2(16.0f, 16.0f));
				break;
			case Suggestion::Type::Class:
				ImGui::Image((ImTextureID)Resources::GetTextureByName("icon-class").descriptorSet, ImVec2(16.0f, 16.0f));
				break;
			}
			ImGui::SameLine();
			if (ImGui::Selectable(suggestion.word.c_str()))
			{
				InsertSuggestion(editor, initialWord, suggestion.word, wordStart, wordEnd);
				s_suggestionData.filteredSuggestions.clear();
			}
		}

		ImGui::EndPopup();
	}
}

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

const TextEditor::Palette &GetTokyoNightPalette()
{
	const static TextEditor::Palette p = { {
			0xFFFAC4C7,	// Default
			0xFFFF8BBC,	// Keyword	
			0xFF52A5EC,	// Number
			0xFF49D5A4,	// String
			0xFF49D5A4, // Char literal
			0xFFE3AD84, // Punctuation
			0xFFFF8BBC,	// Preprocessor
			0xFFFAC4C7, // Identifier
			0xFFFF8BBC, // Known identifier
			0xFFFF948F, // Preproc identifier
			0xFF735457, // Comment (single line)
			0xFF735457, // Comment (multi line)
			0xFF271a1b, // Background
			0xffe0e0e0, // Cursor
			0xFF422c2d, // Selection
			0x800020ff, // ErrorMarker
			0x40f08000, // Breakpoint
			0xFF735457, // Line number
			0x40000000, // Current line fill
			0x40000000, // Current line fill (inactive)
			0x0, // Current line edge
		} };
	return p;
}

std::shared_ptr<EditorWindow> EditorWindow::Create(const std::filesystem::path &filePath)
{
	std::shared_ptr<EditorWindow> window = std::make_shared<EditorWindow>();
	window->m_filePath = filePath;

	auto lang = TextEditor::LanguageDefinition::CPlusPlus();

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
	}

	static const char* identifiers[] = {
		"HWND", "HRESULT", "LPRESULT","D3D11_RENDER_TARGET_VIEW_DESC", "DXGI_SWAP_CHAIN_DESC","MSG","LRESULT","WPARAM", "LPARAM","UINT","LPVOID",
		"ID3D11Device", "ID3D11DeviceContext", "ID3D11Buffer", "ID3D11Buffer", "ID3D10Blob", "ID3D11VertexShader", "ID3D11InputLayout", "ID3D11Buffer",
		"ID3D10Blob", "ID3D11PixelShader", "ID3D11SamplerState", "ID3D11ShaderResourceView", "ID3D11RasterizerState", "ID3D11BlendState", "ID3D11DepthStencilState",
		"IDXGISwapChain", "ID3D11RenderTargetView", "ID3D11Texture2D", "TextEditor",
		"int8_t", "int16_t", "int32_t", "int64_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t", "size_t", "uintptr_t" };
	static const char* idecls[] = 
	{
		"typedef HWND_* HWND", "typedef long HRESULT", "typedef long* LPRESULT", "struct D3D11_RENDER_TARGET_VIEW_DESC", "struct DXGI_SWAP_CHAIN_DESC",
		"typedef tagMSG MSG\n * Message structure","typedef LONG_PTR LRESULT","WPARAM", "LPARAM","UINT","LPVOID",
		"ID3D11Device", "ID3D11DeviceContext", "ID3D11Buffer", "ID3D11Buffer", "ID3D10Blob", "ID3D11VertexShader", "ID3D11InputLayout", "ID3D11Buffer",
		"ID3D10Blob", "ID3D11PixelShader", "ID3D11SamplerState", "ID3D11ShaderResourceView", "ID3D11RasterizerState", "ID3D11BlendState", "ID3D11DepthStencilState",
		"IDXGISwapChain", "ID3D11RenderTargetView", "ID3D11Texture2D", "class TextEditor",
		"typedef signed char int8_t", "typedef short int16_t", "typedef int int32_t", "typedef long long int64_t",
		"typedef unsigned char uint8_t", "typedef unsigned short uint16_t", "typedef unsigned int uint32_t", "typedef unsigned long long uint64_t",
		"typedef unsigned long size_t", "typedef unsigned long uintptr_t" };

	for (int i = 0; i < sizeof(identifiers) / sizeof(identifiers[0]); ++i)
	{
		TextEditor::Identifier id;
		id.mDeclaration = std::string(idecls[i]);
		lang.mIdentifiers.insert(std::make_pair(std::string(identifiers[i]), id));
	}*/

	window->m_editor.SetLanguageDefinition(lang);
	window->m_editor.SetPalette(GetTokyoNightPalette());

    std::ifstream filestream(filePath);
    
    if (filestream.is_open())
	{
		std::ostringstream ss;
		ss << filestream.rdbuf();
		
        window->m_editor.SetText(ss.str());

		filestream.close();
    }

	return window;
}

void EditorWindow::OnWindowAdded(void)
{
	BuildErrorHandler &errorHandler = m_application->GetBuildErrorHandler();
	if (errorHandler.HasAppliedErrorMarkers())
	{
		std::vector<BuildErrorHandler::BuildError> filteredErrors = errorHandler.FilterErrorsByPath(m_filePath);

		if (!filteredErrors.empty())
		{
			TextEditor::ErrorMarkers markers;
			for (const auto &error : filteredErrors)
				markers.insert(std::make_pair(error.m_lineNumber - 1, error.m_errorMessage));
			m_editor.SetErrorMarkers(markers);
		}
	}
}

void EditorWindow::GetCurrentWord(std::string& word, int& start, int& end)
{
    std::string line = m_editor.GetCurrentLineText();
    const int cursorPosition = m_editor.GetCursorPosition().mColumn;

	line = ConvertTabsToSpaces(line, 4);

    if (line.empty())
	{
        word.clear();
        start = end = cursorPosition;
        return;
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
}

void EditorWindow::RenderWindow(void)
{
	std::string title = m_filePath.filename().string() + "##" + m_filePath.string();
	const char *c_title = title.c_str();

	int start, end;
	ImVec2 windowPosition;

	if (ImGui::Begin(c_title, &m_opened))
	{
		ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_Once);
		
		std::string word;
		GetCurrentWord(word, start, end);
		FilterSuggestions(word);

		static bool focusSuggestions = false;
		const bool hasSuggestions = !s_suggestionData.filteredSuggestions.empty();

		if (m_editor.IsFocused() && hasSuggestions)
		{
			if (ImGui::IsKeyPressed(ImGuiKey_Tab))
			{
				m_editor.SetReadOnly(true);
				focusSuggestions = true;
			}
		}

		m_editor.Render(c_title);

		if (m_editor.IsFocused())
			m_application->SetLastWindow(this);

		if (hasSuggestions)
		{
			m_editor.SetReadOnly(false);

			if (m_application->GetLastWindow() == this)
			{
				if (m_editor.IsFocused())
					ImGui::OpenPopup("##Suggestions");
				RenderSuggestions(m_editor, word, ImGui::GetWindowPos(), focusSuggestions, start, end);
			}
		}

		if (m_editor.IsFocused() && ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_S))
		{
			std::ofstream file(m_filePath);
			if (file.is_open())
			{
				std::string text = m_editor.GetText();
				text.pop_back();
				file << text;
				file.close();
			}
		}
		ImGui::End();
	}
}

}