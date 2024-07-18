#pragma once

#include "WindowPlugin.h"
#include "TextEditor.h"

namespace Crystal
{

class EditorWindow : public WindowPlugin
{
public:
	static std::shared_ptr<EditorWindow> Create(const std::filesystem::path &filePath);
	void RenderWindow(void);
	void OnWindowAdded(void);

	std::filesystem::path GetFilePath(void) const { return m_filePath; }
	void SetFilePath(std::filesystem::path path) { m_filePath = path; }

	TextEditor *GetTextEditor(void) { return &m_editor; }

private:
	void GetCurrentWord(std::string &word, int &start, int &end);

	TextEditor m_editor;
	std::filesystem::path m_filePath;
};

}