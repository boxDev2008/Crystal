#pragma once

#include "Window.h"
#include "TextEditor/TextEditor.h"

namespace Crystal
{

class ExplorerWindow : public Window
{
public:
	void RenderWindow(void);

private:
	void RenderBranch(const std::filesystem::path &directory);
	float m_mainDragDropTargetPosition;
};

}