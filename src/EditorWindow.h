#pragma once

#include "Window.h"
#include "WindowPathContainer.h"
#include "TextEditor.h"

namespace Crystal
{

class EditorWindow : public Window, public WindowPathContainer
{
public:
	EditorWindow(const std::filesystem::path &filePath);
	~EditorWindow(void) { }

	void RenderWindow(void);
	void OnWindowAdded(void);

	void SetFilePath(const std::filesystem::path &path);
	void SaveToFile(void);

	TextEditor *GetTextEditor(void) { return &m_editor; }

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

	private:
		TextEditor::Coordinates m_lastCoords{};

		friend class EditorWindow;
	};

	// void GetCurrentWord(std::string &word, int &start, int &end);

	bool m_saved = true;
	TextEditor m_editor;
	FindReplaceHandler m_findReplaceHandler;
};

}