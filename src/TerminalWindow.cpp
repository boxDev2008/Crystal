#include "TerminalWindow.h"
#include "Application.h"

#include <stdio.h>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <future>

#if _WIN32
	#define popen _popen
	#define pclose _pclose
#endif

namespace Crystal
{

std::shared_ptr<TerminalWindow> TerminalWindow::Create(const std::filesystem::path &directory)
{
	std::shared_ptr<TerminalWindow> window = std::make_shared<TerminalWindow>();
	window->m_currentDirectory = directory;
	return window;
}

void TerminalWindow::RenderWindow(void)
{
    if (ImGui::Begin("Terminal"))
    {
        ImGui::BeginChild("##ScrollingRegion", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar);
        for (const auto &line : m_outputBuffer)
            ImGui::TextUnformatted(line.c_str());

        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);

        ImGui::EndChild();

        ImGui::PushItemWidth(-1);

        if (ImGui::IsWindowFocused() && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
            ImGui::SetKeyboardFocusHere();
        
        if (ImGui::InputText("##Input", m_inputBuffer, 256, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            std::string command(m_inputBuffer);
            m_outputBuffer.push_back(m_currentDirectory.string() + "> " + command);
            std::thread(&TerminalWindow::ExecuteCommand, this, command).detach();
            m_inputBuffer[0] = '\0';
            ImGui::SetKeyboardFocusHere(-1);
        }

        ImGui::End();
    }
}

std::string TerminalWindow::ExecuteSingleCommand(const std::string &cmd, int& returnCode)
{
    // Append " 2>&1" to the command to redirect stderr to stdout
    std::string cmdWithRedirect = cmd + " 2>&1";
    
    FILE* pipe = popen(cmdWithRedirect.c_str(), "r");
    if (!pipe) return "Error: popen failed!";
    
    char buffer[128];
    std::string result = "";
    while (fgets(buffer, sizeof(buffer), pipe) != NULL)
        result += buffer;
    
    returnCode = pclose(pipe);
    if (returnCode != 0)
        result += "\nError: Command returned non-zero exit status";
    return result;
}

void TerminalWindow::ExecuteCommand(const std::string &cmd)
{
	namespace fs = std::filesystem;

    std::vector<std::string> commands;
    std::istringstream command_stream(cmd.c_str());
    std::string command;
    while (std::getline(command_stream, command, ';'))
	{
        std::istringstream subCommandStream(command);
        std::string subCommand;
        while (std::getline(subCommandStream, subCommand, '&'))
		{
            if (!subCommand.empty())
                commands.push_back(subCommand);
        }
    }

    std::string result;
    bool success = true;
    
    for (const std::string& cmd : commands)
	{
        std::string trimmedCommand = cmd;
        trimmedCommand.erase(trimmedCommand.find_last_not_of(" \n\r\t")+1);

        if (trimmedCommand.empty()) continue;

        bool runNextCommand = true;
        if (trimmedCommand.back() == '|')
		{
            runNextCommand = !success;
            trimmedCommand.pop_back();
        }
		else if (trimmedCommand.back() == '&')
		{
            runNextCommand = success;
            trimmedCommand.pop_back();
        }

		if (trimmedCommand.find("cd") != std::string::npos)
		{
			std::string::size_type firstChar = trimmedCommand.find_first_not_of(' ');
			if (firstChar != std::string::npos && trimmedCommand.substr(firstChar, 3) == "cd ")
			{
				fs::path previousDirectory = m_currentDirectory;
	
				trimmedCommand = trimmedCommand.substr(3);
				size_t startpos = trimmedCommand.find_first_not_of(" \t");
				m_currentDirectory /= trimmedCommand.substr(startpos);
				m_currentDirectory = fs::weakly_canonical(m_currentDirectory);

				if (!fs::exists(m_currentDirectory))
				{
					result += "cd : Cannot find path \'" + m_currentDirectory.string() + "\' because it does not exist.";
					m_currentDirectory = previousDirectory;
					success = false;
				}
			}
		}
		else if (trimmedCommand.find("cls") != std::string::npos || trimmedCommand.find("clear") != std::string::npos)
		{
			m_outputBuffer.clear();
		}
		else if (trimmedCommand.find("time") != std::string::npos); // NOTE (box): Time is crashing for some reason. This is a quick fix.
		else
		{
            int returnCode = 0;
			std::string command_result = ExecuteSingleCommand("cd " + m_currentDirectory.string() + " && " + trimmedCommand, returnCode);

            if (command_result.find("MSBuild") != std::string::npos || command_result.find("msbuild") != std::string::npos) // Check if a compiler is being used
                m_application->GetBuildErrorHandler().ApplyErrorMarkersToAllEditorWindows(m_application->GetAllWindowPlugins(),
                command_result, BuildErrorHandler::CompilerType::MSBuild);

			result += command_result;

			//if (command_result.find("Error:") != std::string::npos)
            if (returnCode != 0)
				success = false;
		}
        
        if (!runNextCommand)
            break;
    }

	m_outputBuffer.push_back(result);
}

}