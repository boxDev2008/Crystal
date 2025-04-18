#pragma once

#include "Window.h"
#include "WindowPathContainer.h"
#include "TextEditor/TextEditor.h"

#include <tree_sitter/api.h>

namespace Crystal
{

class EditorWindow : public Window, public WindowPathContainer
{
public:
	EditorWindow(const std::filesystem::path &filePath);
	~EditorWindow(void);

	void RenderWindow(void);
	void OnWindowAdded(void);

	void SetFilePath(const std::filesystem::path &path);
	void SaveToFile(void);

	TextEditor *GetTextEditor(void) { return &m_editor; }

protected:
	void GetCurrentWord(std::string &word, int &start, int &end);

private:
	class FindReplaceHandler
	{
	public:
		void FindNext(TextEditor &editor, const std::string &word);
		void FindPrevious(TextEditor &editor, const std::string &word);
		void Replace(TextEditor &editor, const std::string &word, const std::string &replacement);
		void ReplaceAll(TextEditor &editor, const std::string &word, const std::string &replacement);
	protected:
		void Render(TextEditor &editor);

		bool m_active = false;

	private:
		TextEditor::Coordinates m_lastCoords{};

		friend class EditorWindow;
	};

	char m_titleBuffer[1024];

	TextEditor m_editor;
	FindReplaceHandler m_findReplaceHandler;
	TSTree *m_tree = nullptr;
	TSParser *m_parser = nullptr;
};

}