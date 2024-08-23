#include "WizardWindow.h"
#include "Application.h"
#include "Resources.h"
#include "ImGuiFileDialog.h"
#include "EditorWindow.h"
#include "ImageWindow.h"

namespace Crystal
{

void WizardWindow::RenderWindow(void)
{
	WindowManager &windowManager = m_application->GetWindowManager();
	if (windowManager.CheckForWindowOfType<EditorWindow>() ||
		windowManager.CheckForWindowOfType<ImageWindow>())
	{
		m_opened = false;
		return;
	}

	ImGui::SetNextWindowDockID(m_application->GetLayoutHandler().GetMainDockID(), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("##Wizard", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove))
	{
		Texture icon = Resources::GetTextureByName("logo-small");
		float aspectRatio = icon.width / (float)icon.height;
		float iconWidth = 100 * aspectRatio;
		float iconHeight = 100;

		float windowWidth = ImGui::GetWindowSize().x;
		float windowHeight = ImGui::GetWindowSize().y;

		float imageX = (windowWidth - iconWidth) * 0.5f;
		float imageY = (windowHeight - iconHeight) * 0.5f - 20;

		ImGui::SetCursorPos(ImVec2(imageX, imageY));
		ImGui::Image((ImTextureID)icon.id, ImVec2(iconWidth, iconHeight));

		float textX = (windowWidth - ImGui::CalcTextSize("Drag files or directory here").x) * 0.5f;
		float textY = imageY + iconHeight + 20;

		ImGui::SetCursorPos(ImVec2(textX, textY));
		ImGui::TextUnformatted("Drag files or directory here");
	}
	ImGui::End();
}

}