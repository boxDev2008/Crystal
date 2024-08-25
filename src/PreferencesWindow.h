#pragma once

#include "Window.h"
#include "Rendering/Renderer.h"

namespace Crystal
{

class PreferencesWindow : public Window
{
public:
	void RenderWindow(void);

private:
	int32_t m_selectedTheme = -1;
	int32_t m_selectedFont = -1;
};

}