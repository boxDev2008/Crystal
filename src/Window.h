#pragma once

#include <iostream>
#include <filesystem>

namespace Crystal
{

class Application;
class WindowManager;

class Window
{
public:
	virtual ~Window(void) { }
	virtual void OnWindowAdded(void) { }
	virtual void RenderWindow(void) { }

protected:
	bool m_opened = true;
	Application *m_application;

	friend class WindowManager;
};

}