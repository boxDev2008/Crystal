#include "EditorWindow.h"
#include "Application.h"

#include <fstream>
#include <sstream>
#include <iostream>

namespace Crystal
{

const TextEditor::Palette &GetTokyoNightPalette()
{
	const static TextEditor::Palette p = { {
			0xFFF1CAC2,	// Default
			0xFFF19BB6,	// Keyword	
			0xFF6FA2F1,	// Number
			0xFF76CDA8,	// String
			0xFF76CDA8, // Char literal
			0xFFF1A182, // Punctuation
			0xFFF19BB6,	// Preprocessor
			0xFFF1CAC2, // Identifier
			0xFFF19BB6, // Known identifier
			0xFFF1A182, // Preproc identifier
			0xFF7A5953, // Comment (single line)
			0xFF7A5953, // Comment (multi line)
			0x0, // Background
			0xffe0e0e0, // Cursor
			0xFFDB5E87, // Selection
			0x800020ff, // ErrorMarker
			0x40f08000, // Breakpoint
			0xFF7A5953, // Line number
			0x40000000, // Current line fill
			0x40808080, // Current line fill (inactive)
			0x0, // Current line edge
		} };
	return p;
}


std::shared_ptr<EditorWindow> EditorWindow::Create(std::filesystem::path &file)
{
	std::shared_ptr<EditorWindow> window = std::make_shared<EditorWindow>();
	window->m_filePath = file;

	auto lang = TextEditor::LanguageDefinition::CPlusPlus();

	static const char* ppnames[] = { "NULL", "PM_REMOVE",
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
	}
	window->m_editor.SetLanguageDefinition(lang);

	/*TextEditor::ErrorMarkers markers;
	markers.insert(std::make_pair<int, std::string>(6, "Example error here:\nInclude file not found: \"TextEditor.h\""));
	markers.insert(std::make_pair<int, std::string>(41, "Another example error"));
	window->editor.SetErrorMarkers(markers);*/

	window->m_editor.SetPalette(GetTokyoNightPalette());

    std::ifstream filestream(file);
    
    if (filestream.is_open())
	{
		std::ostringstream ss;
		ss << filestream.rdbuf();
		
        window->m_editor.SetText(ss.str());

		filestream.close();
    }

	return window;
}

void EditorWindow::RenderWindow(void)
{
	std::string title = m_filePath.filename().string();
	const char *c_title = title.c_str();

	if (ImGui::Begin(c_title, &m_opened))
	{
		ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_Once);

		m_editor.Render(c_title);

		//if (ImGui::IsWindowFocused())
			//m_application->SetLastWindow(shared_from_this());

		if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_S))
		{
			std::ofstream file(m_filePath);
			if (file.is_open())
			{
				file << m_editor.GetText();
				file.close();
				std::cout << "File saved successfully!" << std::endl;
			}
		}
		ImGui::End();
	}
}

}