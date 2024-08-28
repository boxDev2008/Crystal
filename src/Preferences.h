#pragma once

#include "TextEditor.h"
#include "WindowManager.h"
#include "PlatformWindow.h"

namespace Crystal
{

class Preferences
{
public:
    class FontPack
    {
    public:
        ImFont* GetClosestFont(float fontSize);
        ImFont *m_fonts[4];
    };

    class GlobalSettings
    {
    public:
        void SetColorTheme(const std::string &name);
        const std::vector<const char*> GetColorThemeNameList(void) const;
        const TextEditor::Palette &GetEditorPalette(void) const { return m_editorPalette; }

        void SetFont(const std::string &name);
        const std::vector<const char*> GetFontNameList(void) const;

        float fontSize = 20.0f;

    protected:
        void RefreshColorThemeList(void);

        PlatformWindow *m_mainWindow;
        WindowManager *m_wm;

    private:
        FontPack *m_fontPack;
        std::unordered_map<std::string, FontPack> m_fontPacks;
        std::string m_fontName;

        std::unordered_map<std::string, std::filesystem::path> m_colorThemes;
        std::string m_colorThemeName;
        TextEditor::Palette m_editorPalette;

        friend class Preferences;
    };

    class UISettings
    {
    public:
        float indentSpacing = 16.0f;
    };

    class EditorSettings
    {
    public:
        int32_t tabSize = 4;
        float lineSpacing = 1.0f;
        bool showLineNumbers = false;
        bool showWhitespaces = false;
        bool shortTabs = false;
        bool autoIndent = true;
        bool smoothScroll = false;
        float smoothScrollSpeed = 20.0f;
    };

    Preferences(void) = default;
    Preferences(PlatformWindow *platformWindow, WindowManager &wm);

    ~Preferences(void);

    void Refresh(void);

    GlobalSettings &GetGlobalSettings(void) { return m_globalSettings; }
    UISettings &GetUISettings(void) { return m_uiSettings; }
    EditorSettings &GetEditorSettings(void) { return m_editorSettings; }

private:

    GlobalSettings m_globalSettings;
    UISettings m_uiSettings;
    EditorSettings m_editorSettings;
};

}