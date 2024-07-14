#pragma once

#include "WindowPlugin.h"
#include "TextEditor.h"

namespace Crystal
{

class EditorWindow : public WindowPlugin
{
public:
	static std::shared_ptr<EditorWindow> Create(std::filesystem::path &file);
	void RenderWindow(void);
	std::filesystem::path GetFilePath(void) { return m_filePath; }

private:
	TextEditor m_editor;
	std::filesystem::path m_filePath;
};

}