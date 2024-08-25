#include "PreferencesWindow.h"
#include "Application.h"
#include "EditorWindow.h"

#include <algorithm>

namespace Crystal
{

void PreferencesWindow::RenderWindow(void)
{
    using Vector2 = Crystal::Math::Vector2;
    Preferences &preferences = m_application->GetPreferences();

    ImGui::SetNextWindowSize(Vector2(600, 700), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Preferences", &m_opened, 0))
    {
        /*ImGui::BeginChild("LeftPanel", Vector2(ImGui::GetWindowSize().x * 0.25f, 0), true);
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("RightPanel", ImVec2(0, 0), false);*/

        ImGuiIO &io = ImGui::GetIO();
        ImGuiStyle &style = ImGui::GetStyle();

        const ImColor headerColor = ImColor(preferences.GetGlobalSettings().GetEditorPalette()[1]);
        ImGui::TextColored(headerColor, "Global Settings");
        {
            Preferences::GlobalSettings &settings = preferences.GetGlobalSettings();
            ImGui::TextUnformatted("Font Size");
            ImGui::DragFloat("##FontSize", &settings.fontSize, 0.1f, 13.0f, 64.0f, "%.1f");
            settings.fontSize = std::clamp(settings.fontSize, 13.0f, 64.0f);

            std::vector<const char*> fonts = settings.GetFontNameList();
            ImGui::TextUnformatted("Fonts");
            if (ImGui::Combo("##Fonts", &m_selectedFont, fonts.data(), fonts.size()))
                settings.SetFont(fonts[m_selectedFont]);

            std::vector<const char*> themes = settings.GetColorThemeNameList();
            ImGui::TextUnformatted("Theme");
            if (ImGui::Combo("##Theme", &m_selectedTheme, themes.data(), themes.size()))
                settings.SetColorTheme(themes[m_selectedTheme]);
        }

        ImGui::TextColored(headerColor, "UI Settings");
        {
            Preferences::UISettings &settings = preferences.GetUISettings();
            ImGui::TextUnformatted("Indent Spacing");
            ImGui::DragFloat("##IndentSpacing", &settings.indentSpacing, 0.1f, 0.0f, FLT_MAX, "%.1f");
        }

        ImGui::TextColored(headerColor, "Editor Settings");
        {
            Preferences::EditorSettings &settings = preferences.GetEditorSettings();
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, style.FramePadding.y * 0.60f));
            ImGui::Checkbox("Show Line Numbers", &settings.showLineNumbers);
            ImGui::Checkbox("Show Whitespaces", &settings.showWhitespaces);
            if (settings.showWhitespaces)
            {
                ImGui::SameLine();
                ImGui::Checkbox("Short Tabs", &settings.shortTabs);
            }
            ImGui::Checkbox("Auto Indent", &settings.autoIndent);
            ImGui::Checkbox("Smooth Scroll (Experimental)", &settings.smoothScroll);
            ImGui::PopStyleVar();

            ImGui::TextUnformatted("Tab Size");
            ImGui::DragInt("##TabSize", &settings.tabSize, 0.1f, 1, 32);
            settings.tabSize = std::clamp(settings.tabSize, 1, 32);

            ImGui::TextUnformatted("Line Spacing");
            ImGui::DragFloat("##LineSpacing", &settings.lineSpacing, 0.01f, 0.5f, 2.0f, "%.2f");
            settings.lineSpacing = std::clamp(settings.lineSpacing, 0.5f, 2.0f);
        }

        //ImGui::EndChild();
    }
    ImGui::End();
}

}