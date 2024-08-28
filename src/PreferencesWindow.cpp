#include "PreferencesWindow.h"
#include "Application.h"
#include "EditorWindow.h"

#include "Math/Math.h"

namespace Crystal
{

void PreferencesWindow::RenderWindow(void)
{
	using namespace Crystal::Math;

    Preferences &preferences = m_application->GetPreferences();

    //ImGui::SetNextWindowSize(ImVec2(600, 700), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowDockID(m_application->GetLayoutHandler().GetMainDockID(), ImGuiCond_Appearing);
    if (ImGui::Begin("Preferences##Preferences", &m_opened, 0))
    {
        /*ImGui::BeginChild("LeftPanel", ImVec2(ImGui::GetWindowSize().x * 0.25f, 0), true);
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

        ImGui::NewLine();
        ImGui::TextColored(headerColor, "UI Settings");
        {
            Preferences::UISettings &settings = preferences.GetUISettings();
            ImGui::TextUnformatted("Indent Spacing");
            ImGui::DragFloat("##IndentSpacing", &settings.indentSpacing, 0.1f, 0.0f, FLT_MAX, "%.1f");
        }

        ImGui::NewLine();
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
            if (settings.smoothScroll)
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f - ImGui::CalcTextSize("Scroll Speed").x);
                ImGui::DragFloat("Scroll Speed", &settings.smoothScrollSpeed, 0.1f, 1.0f, 50.0f, "%.1f");
                ImGui::PopItemWidth();
                settings.smoothScrollSpeed = Clamp(settings.smoothScrollSpeed, 1.0f, 50.0f);
            }
            ImGui::PopStyleVar();

            ImGui::TextUnformatted("Tab Size");
            ImGui::DragInt("##TabSize", &settings.tabSize, 0.1f, 1, 32);
            settings.tabSize = Clamp(settings.tabSize, 1, 32);

            ImGui::TextUnformatted("Line Spacing");
            ImGui::DragFloat("##LineSpacing", &settings.lineSpacing, 0.01f, 0.5f, 2.0f, "%.2f");
            settings.lineSpacing = Clamp(settings.lineSpacing, 0.5f, 2.0f);
        }

        //ImGui::EndChild();
    }
    ImGui::End();
}

}