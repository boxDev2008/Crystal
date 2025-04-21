#pragma once

#include "imgui.h"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <functional>

namespace Crystal
{

struct PlatformWindowSettings
{
    uint32_t width, height;
    uint32_t minWidth, minHeight;

    const char *title;
    const char *iconPath;

    static const PlatformWindowSettings Default(void) { return { 1440, 900, 400, 200, "Crystal", "res/icon.png" }; };
};

class PlatformWindow
{
public:
    struct WindowData
    {
        int32_t x, y;
        int32_t width, height;
        int32_t fbWidth, fbHeight;
        int32_t mouseX, mouseY;
        bool shouldClose = false;
        bool resizing = false;
        bool moving = false;
		bool maxmizing = false;
        std::function<void()> onForceRedrawCallback;
        std::function<void(int32_t count, const char **paths)> onDragFilesCallback;
    };

    PlatformWindow(void) = default;
    PlatformWindow(
        const PlatformWindowSettings &settings,
        const std::function<void()> &onForceRedrawCallback = nullptr,
        const std::function<void(int32_t count, const char **paths)> &onDragFilesCallback = nullptr);

    ~PlatformWindow(void);

    //void SetColors(unsigned long titlebar, unsigned long border, unsigned long text);
	void SetBorderColor(unsigned long color);
	void SetCaptionHeight(int32_t value);

	void Maximize(void) { IsMaximized() ? glfwRestoreWindow(m_window) : glfwMaximizeWindow(m_window); m_data.maxmizing = false; }
	void Minimize(void) { glfwIconifyWindow(m_window); }
	void Close(void) { m_data.shouldClose = true; }

    GLFWwindow *GetGlfwHandle(void) { return m_window; }

    const ImVec2 GetSize(void) const { return ImVec2(m_data.width, m_data.height); }
    const ImVec2 GetPosition(void) const { return ImVec2(m_data.x, m_data.y); }
    const ImVec2 GetFramebufferSize(void) const { return ImVec2(m_data.fbWidth, m_data.fbHeight); }
    const ImVec2 GetMousePosition(void) const { return ImVec2(m_data.mouseX, m_data.mouseY); }
    
    bool IsRunning(void) { return !m_data.shouldClose; }
    bool IsResizing(void) { return m_data.resizing; }
    bool IsMoving(void) { return m_data.moving; }
    bool IsMaximized(void) { return glfwGetWindowAttrib(m_window, GLFW_MAXIMIZED); }

protected:
    void PollEvents(void);

private:
    GLFWwindow *m_window;
    WindowData m_data;

    friend class Application;
};

}