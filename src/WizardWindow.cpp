#include "WizardWindow.h"
#include "Application.h"
#include "Resources.h"
#include "ImGuiFileDialog.h"
#include "ExplorerWindow.h"
#include "TerminalWindow.h"

namespace Crystal
{

std::shared_ptr<WizardWindow> WizardWindow::Create(void)
{
	std::shared_ptr<WizardWindow> window = std::make_shared<WizardWindow>();
	return window;
}

void WizardWindow::RenderWindow(void)
{
	if (ImGui::Begin("Welcome Wizard", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking))
	{
		ImVec2 mainWindowSize = m_application->GetGlfwWindowSize();
		ImVec2 mainWindowOffset = m_application->GetGlfwWindowPosition();

		ImGui::SetWindowPos(ImVec2(mainWindowSize.x * 0.5f + mainWindowOffset.x - 300, mainWindowSize.y * 0.5f + mainWindowOffset.y - 250), ImGuiCond_Always);
		ImGui::SetWindowSize(ImVec2(600, 500), ImGuiCond_Once);

		VulkanContext::Texture icon = Resources::GetTextureByName("logo-mid");
		float aspectRatio = icon.width / (float)icon.height;
		ImGui::SetCursorPos(ImVec2((ImGui::GetWindowSize().x - 100 * aspectRatio) * 0.5f, 80.0f));
		ImGui::Image((ImTextureID)icon.descriptorSet, ImVec2(100 * aspectRatio, 100));
		ImGui::NewLine();ImGui::NewLine();ImGui::NewLine();

    	//ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("New Directory").x + ImGui::GetStyle().FramePadding.x * 2) * 0.5f);
		//ImGui::Button("New Directory");
    	ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("Open Directory").x + ImGui::GetStyle().FramePadding.x * 2) * 0.5f);
		if (ImGui::Button("Open Directory"))
		{
			IGFD::FileDialogConfig config;
			config.path = ".";
			config.countSelectionMax = 1;
    		ImGuiFileDialog::Instance()->OpenDialog("ChooseDirectoryDlgKey", "Choose Directory", nullptr, config);
		}
		ImGui::End();

		if (ImGuiFileDialog::Instance()->Display("ChooseDirectoryDlgKey"))
		{
			if (ImGuiFileDialog::Instance()->IsOk())
			{
				//std::string path = ImGuiFileDialog::Instance()->GetFilePathName();
				std::string path = ImGuiFileDialog::Instance()->GetCurrentPath();
				m_application->SetMainDirectoryPath(path);

				m_application->AddWindowPlugin(ExplorerWindow::Create(path));
				m_application->AddWindowPlugin(TerminalWindow::Create(path));
				m_opened = false;
			}
			
			ImGuiFileDialog::Instance()->Close();
		}
	}
}

}