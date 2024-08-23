#pragma once

#include "Window.h"
#include <vector>

namespace Crystal
{

class Application;
class WindowManager
{
public:
    WindowManager(void) = default;
    WindowManager(Application *application)
    {
        m_application = application;
    }

	void RenderAllWindows(void);
	void ManageFreedCache(void);
	void FreeAllWindows(void);

	std::vector<Window*> &GetAllWindows(void) { return m_windows; }
	void AddWindow(Window *window);

	Window *CheckForWindowWithPath(const std::filesystem::path &path);

	template <typename T>
	T *CheckForWindowOfType(void)
	{
		static_assert(std::is_base_of<Window, T>::value, "T must extend WindowPlugin");

		for (Window *window : m_windows)
			if (T *specificWindow = dynamic_cast<T*>(window))
				return specificWindow;

		return nullptr;
	}

	template <typename T, typename Callable>
	void ForEachWindowOfType(Callable func)
	{
		static_assert(std::is_base_of<Window, T>::value, "T must extend Window");

		for (Window *window : m_windows)
			if (T *specificWindow = dynamic_cast<T*>(window))
				func(specificWindow);
	}

	void SetLastWindow(Window *window) { m_lastWindow = window; }
	Window *GetLastWindow(void) const { return m_lastWindow; }

private:
	std::vector<Window*> m_windows;
	std::vector<Window*> m_freedWindows;
	Window *m_lastWindow = nullptr;
    Application *m_application;
};

}