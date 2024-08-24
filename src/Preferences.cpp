#include "Preferences.h"
#include "EditorWindow.h"
#include "Utils.h"
#include "math/Lerp.h"

#include <mini/ini.h>

#include <iostream>
#include <filesystem>

namespace Crystal
{

static std::unordered_map<ImGuiCol, const char*> s_imguiColorMappings = {
    {ImGuiCol_Text, "TextPrimary"},
    {ImGuiCol_TextDisabled, "TextDisabled"},
    {ImGuiCol_WindowBg, "WindowBg"},
    {ImGuiCol_ChildBg, "ChildBg"},
    {ImGuiCol_PopupBg, "PopupBg"},
    {ImGuiCol_Border, "Border"},
    {ImGuiCol_FrameBg, "FrameBg"},
    {ImGuiCol_FrameBgHovered, "FrameBgHovered"},
    {ImGuiCol_FrameBgActive, "FrameBgActive"},
    {ImGuiCol_TitleBg, "TitleBg"},
    {ImGuiCol_TitleBgActive, "TitleBgActive"},
    {ImGuiCol_TitleBgCollapsed, "TitleBgCollapsed"},
    {ImGuiCol_MenuBarBg, "MenuBarBg"},
    {ImGuiCol_ScrollbarBg, "ScrollbarBg"},
    {ImGuiCol_ScrollbarGrab, "ScrollbarGrab"},
    {ImGuiCol_ScrollbarGrabHovered, "ScrollbarGrabHovered"},
    {ImGuiCol_ScrollbarGrabActive, "ScrollbarGrabActive"},
    {ImGuiCol_CheckMark, "CheckMark"},
    {ImGuiCol_SliderGrab, "SliderGrab"},
    {ImGuiCol_SliderGrabActive, "SliderGrabActive"},
    {ImGuiCol_Button, "Button"},
    {ImGuiCol_ButtonHovered, "ButtonHovered"},
    {ImGuiCol_ButtonActive, "ButtonActive"},
    {ImGuiCol_Header, "Header"},
    {ImGuiCol_HeaderHovered, "HeaderHovered"},
    {ImGuiCol_HeaderActive, "HeaderActive"},
    {ImGuiCol_Separator, "Separator"},
    {ImGuiCol_SeparatorHovered, "SeparatorHovered"},
    {ImGuiCol_SeparatorActive, "SeparatorActive"},
    {ImGuiCol_ResizeGrip, "ResizeGrip"},
    {ImGuiCol_ResizeGripHovered, "ResizeGripHovered"},
    {ImGuiCol_ResizeGripActive, "ResizeGripActive"},
    {ImGuiCol_Tab, "Tab"},
    {ImGuiCol_TabHovered, "TabHovered"},
    {ImGuiCol_TabSelected, "TabSelected"},
    {ImGuiCol_TabSelectedOverline, "TabSelectedOverline"},
    {ImGuiCol_TabDimmed, "TabDimmed"},
    {ImGuiCol_TabDimmedSelected, "TabDimmedSelected"},
    {ImGuiCol_TabDimmedSelectedOverline, "TabDimmedSelectedOverline"},
    {ImGuiCol_DockingPreview, "DockingPreview"},
    {ImGuiCol_DockingEmptyBg, "DockingEmptyBg"},
    {ImGuiCol_TextSelectedBg, "TextSelectedBg"},
    {ImGuiCol_NavHighlight, "NavHighlight"},
    {ImGuiCol_ModalWindowDimBg, "ModalWindowDimBg"},
    {ImGuiCol_DragDropTarget, "DragDropTarget"}
};

static ImVec4 hexToColor(const std::string& hex)
{
    if (hex.size() < 8)
        return ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    int32_t r, g, b, a;

    std::istringstream(hex.substr(0,2)) >> std::hex >> r;
    std::istringstream(hex.substr(2,2)) >> std::hex >> g;
    std::istringstream(hex.substr(4,2)) >> std::hex >> b;
    std::istringstream(hex.substr(6,2)) >> std::hex >> a;

    return ImVec4(r * 0.00392f, g * 0.00392f, b * 0.00392f, a * 0.00392f);
}

#if CRYSTAL_PLATFORM_WINDOWS

static unsigned long hexToPlatformColor(const std::string& hex)
{
    if (hex.size() < 8)
        return 0;

    int32_t r, g, b, a;

    std::istringstream(hex.substr(0,2)) >> std::hex >> r;
    std::istringstream(hex.substr(2,2)) >> std::hex >> g;
    std::istringstream(hex.substr(4,2)) >> std::hex >> b;
    std::istringstream(hex.substr(6,2)) >> std::hex >> a;
    
    unsigned long abgr = (static_cast<unsigned long>(0) << 24) |
                         (static_cast<unsigned long>(b) << 16) |
                         (static_cast<unsigned long>(g) << 8) |
                         (static_cast<unsigned long>(r));

    return abgr;
}

#endif

Preferences::Preferences(PlatformWindow *mainWindow, WindowManager &wm)
{
    m_globalSettings.m_mainWindow = mainWindow;
    m_globalSettings.m_wm = &wm;

    m_globalSettings.RefreshColorThemeList();
}

const std::vector<const char*> Preferences::GlobalSettings::GetColorThemeNameList(void) const
{
    std::vector<const char*> keys;
    keys.reserve(m_colorThemes.size());

    static std::vector<std::string> stringStorage;

    stringStorage.clear();

    for (const auto& pair : m_colorThemes)
    {
        stringStorage.push_back(pair.first);
        keys.push_back(stringStorage.back().c_str());
    }

    return keys;
}

void Preferences::GlobalSettings::RefreshColorThemeList(void)
{
	namespace fs = std::filesystem;

    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->Clear();

    m_colorThemes.clear();
    m_fontPacks.clear();

	if (fs::exists("themes") && fs::is_directory("themes"))
        for (const auto &entry : fs::directory_iterator("themes"))
        {
            fs::path path = entry.path();

            if (!fs::is_regular_file(path))
                continue;
            
            fs::path name = path;
            name.replace_extension();
            m_colorThemes.insert({name.filename().string(), path});
        }

	if (fs::exists("fonts") && fs::is_directory("fonts"))
    {
        ImFontConfig fontConfig{};
        fontConfig.OversampleV = 2;
        fontConfig.OversampleH = 2;
        io.Fonts->Clear();
        //fontConfig.RasterizerDensity = 0.5f;
        for (const auto &entry : fs::directory_iterator("fonts"))
        {
            fs::path path = entry.path();

            if (!fs::is_regular_file(path))
                continue;
            
            fs::path name = path;
            name.replace_extension();

            FontPack pack{};
            const char *pathStr = path.string().c_str();
            pack.m_fonts[0] = io.Fonts->AddFontFromFileTTF(pathStr, 13.0f, &fontConfig);
            pack.m_fonts[1] = io.Fonts->AddFontFromFileTTF(pathStr, 20.0f, &fontConfig);
            pack.m_fonts[2] = io.Fonts->AddFontFromFileTTF(pathStr, 32.0f, &fontConfig);
            pack.m_fonts[3] = io.Fonts->AddFontFromFileTTF(pathStr, 48.0f, &fontConfig);

            m_fontPacks.insert({name.filename().string(), pack});
        }
    }
}

void Preferences::GlobalSettings::SetColorTheme(const std::string &name)
{
    mINI::INIFile file(m_colorThemes[name].string());
    mINI::INIStructure ini;
    file.read(ini);

	ImVec4 *imColors = ImGui::GetStyle().Colors;

    for (const auto &[imguiCol, iniKey] : s_imguiColorMappings)
        imColors[imguiCol] = hexToColor(ini["UserInterface"][iniKey]);

#define EDITOR_COLOR(_name) ImColor(hexToColor(ini["Editor"][_name]))

    m_editorPalette = {
        EDITOR_COLOR("Default"),
        EDITOR_COLOR("Keyword"),
        EDITOR_COLOR("Number"),
        EDITOR_COLOR("String"),
        EDITOR_COLOR("CharLiteral"),
        EDITOR_COLOR("Punctuation"),
        EDITOR_COLOR("Preprocessor"),
        EDITOR_COLOR("Identifier"),
        EDITOR_COLOR("KnownIdentifier"),
        EDITOR_COLOR("PreprocIdentifier"),
        EDITOR_COLOR("CommentSingleLine"),
        EDITOR_COLOR("CommentMultiLine"),
        EDITOR_COLOR("Background"),
        EDITOR_COLOR("Cursor"),
        EDITOR_COLOR("Selection"),
        EDITOR_COLOR("ErrorMarker"),
        EDITOR_COLOR("ControlCharacter"),
        EDITOR_COLOR("Breakpoint"),
        EDITOR_COLOR("LineNumber"),
        EDITOR_COLOR("CurrentLineFill"),
        EDITOR_COLOR("CurrentLineFillInactive"),
        EDITOR_COLOR("CurrentLineEdge")
    };

#define PLATFORM_COLOR(_name) hexToPlatformColor(ini["Platform"][_name])

#if CRYSTAL_PLATFORM_WINDOWS
    m_mainWindow->SetColors(
        PLATFORM_COLOR("Titlebar"),
        PLATFORM_COLOR("Border"),
        PLATFORM_COLOR("Text")
    );
#endif

    m_wm->ForEachWindowOfType<EditorWindow>([&](EditorWindow *window) {
        TextEditor *editor = window->GetTextEditor();
        editor->SetPalette(m_editorPalette);
    });
}

void Preferences::GlobalSettings::SetFont(const std::string &name)
{
    m_fontPack = &m_fontPacks[name];
}

const std::vector<const char*> Preferences::GlobalSettings::GetFontNameList(void) const
{
    std::vector<const char*> keys;
    keys.reserve(m_fontPacks.size());

    static std::vector<std::string> stringStorage;

    stringStorage.clear();

    for (const auto& pair : m_fontPacks)
    {
        stringStorage.push_back(pair.first);
        keys.push_back(stringStorage.back().c_str());
    }

    return keys;
}

ImFont* Preferences::FontPack::GetClosestFont(float fontSize)
{
    static const float sizes[] = {13.0f, 20.0f, 32.0f, 48.0f};

    ImFont* closestFont = m_fonts[0];
    float minDiff = std::abs(fontSize - sizes[0]);

    for (int i = 1; i < 4; ++i)
    {
        float diff = std::abs(fontSize - sizes[i]);
        if (diff < minDiff)
        {
            minDiff = diff;
            closestFont = m_fonts[i];
        }
    }

    return closestFont;
}

void Preferences::Refresh(void)
{
    ImGuiIO &io = ImGui::GetIO();
    ImGuiStyle &style = ImGui::GetStyle();

    io.FontDefault = m_globalSettings.m_fontPack->GetClosestFont(m_globalSettings.fontSize);

    io.FontGlobalScale = m_globalSettings.fontSize / io.FontDefault->FontSize;
    style.IndentSpacing = m_uiSettings.indentSpacing;
}

}