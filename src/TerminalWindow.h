#pragma once

#include "WindowPlugin.h"
#include "VulkanContext.h"
#include "TextEditor.h"

namespace Crystal
{

class TerminalWindow : public WindowPlugin
{
public:
	static std::shared_ptr<TerminalWindow> Create(const std::filesystem::path &directory);
	void RenderWindow(void);
    
private:
    void ExecuteCommand(const std::string &cmd);
    std::string ExecuteSingleCommand(const std::string &cmd, int& returnCode);

    std::vector<std::string> m_outputBuffer;
    // std::vector<std::string> m_savedCommands; TODO (box): this

    std::filesystem::path m_currentDirectory;
    char m_inputBuffer[256];
};

}