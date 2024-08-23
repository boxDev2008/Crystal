#include "WindowManager.h"
#include "WindowPathContainer.h"

#include <algorithm>

namespace Crystal
{

Window *WindowManager::CheckForWindowWithPath(const std::filesystem::path &path)
{
	for (Window *window : m_windows)
	{
		WindowPathContainer *pathContainer = dynamic_cast<WindowPathContainer*>(window);

		if (pathContainer)
		{
			if (std::filesystem::equivalent(pathContainer->GetFilePath(), path))
				return window;
			continue;
		}
	}

	return nullptr;
}

void WindowManager::AddWindow(Window *window)
{
	window->m_application = m_application;
	window->OnWindowAdded();
	m_windows.push_back(window);
}

void WindowManager::ManageFreedCache(void)
{
	if (m_freedWindows.empty())
		return;
	
	for (Window *window : m_freedWindows)
	{
		m_windows.erase(std::remove(m_windows.begin(), m_windows.end(), window), m_windows.end());
		delete window;
	}

	m_freedWindows.clear();
}

void WindowManager::FreeAllWindows(void)
{
	for (Window *window : m_windows)
		delete window;

	m_windows.clear();
}

void WindowManager::RenderAllWindows(void)
{
	for (uint32_t i = 0; i < m_windows.size(); i++)
	{
		Window *window = m_windows[i];
		if (!window)
			continue;

		if (!window->m_opened)
		{
			m_freedWindows.push_back(window);
			continue;
		}

		window->RenderWindow();
	}
}


}