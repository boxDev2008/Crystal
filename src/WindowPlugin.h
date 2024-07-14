#pragma once

#include <memory>
#include <iostream>
#include <filesystem>

namespace Crystal
{

class Application;
class WindowPlugin : public std::enable_shared_from_this<WindowPlugin>
{
public:
	virtual void OnWindowAdded(void) { }
	virtual void RenderWindow(void) { }

protected:
	bool m_opened = true;
	Application *m_application;

	friend class Application;
};

}