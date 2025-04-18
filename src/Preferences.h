#pragma once

#include "TextEditor/TextEditor.h"
#include "WindowManager.h"
#include "PlatformWindow.h"

namespace Crystal
{

class Preferences
{
public:

    class GlobalSettings
    {
    public:
        void SetColorTheme(const std::string &name);
        const char* GetCurrentColorThemeName(void) const;
        unsigned int GetColorThemeIndex(const char* fontName) const;
        const std::vector<const char*> GetColorThemeNameList(void) const;
        const TextEditor::Palette &GetEditorPalette(void) const { return m_editorPalette; }

        void SetFont(const std::string &name);
        const char* GetCurrentFontName(void) const;
        const std::vector<const char*> GetFontNameList(void) const;
        unsigned int GetFontIndex(const char* fontName) const;

        float fontSize = 18.0f;
        float editorFontSize = 19.0f;

    protected:
        void RefreshColorThemeList(void);

        PlatformWindow *m_mainWindow;
        WindowManager *m_wm;

    private:
        std::unordered_map<std::string, ImFont*> m_fonts;
        ImFont *m_font;
        std::string m_fontName;

        std::unordered_map<std::string, std::filesystem::path> m_colorThemes;
        std::string m_colorThemeName;
        TextEditor::Palette m_editorPalette;

        friend class Preferences;
    };

    class UISettings
    {
    public:
        float indentSpacing = 8.0f;
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

    void LoadPreferences(char* path);

    GlobalSettings m_globalSettings;
    UISettings m_uiSettings;
    EditorSettings m_editorSettings;
};

}