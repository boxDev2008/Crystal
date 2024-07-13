#pragma once

#include "WindowPlugin.h"
#include "TextEditor.h"

namespace Crystal
{

class ExplorerWindow : public WindowPlugin
{
public:
	static std::shared_ptr<ExplorerWindow> Create(std::filesystem::path &directory);
	std::filesystem::path &GetDirectory(void) { return m_mainDirectoryPath; }
	void RenderWindow(void);

private:
	void RenderBranch(std::filesystem::path &directory);

	std::filesystem::path m_mainDirectoryPath;
};

}