#pragma once

#include "imgui.h"
#include "Math/Vector2.h"

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

    static const PlatformWindowSettings Default(void) { return { 1440, 900, 400, 200, "Crystal", "res/logo-small.png" }; };
};

class PlatformWindow
{
    using Vector2 = Crystal::Math::Vector2;

public:
    struct WindowData
    {
        int32_t x, y;
        int32_t width, height;
        int32_t fbWidth, fbHeight;
        int32_t mouseX, mouseY;
        bool shouldClose = false;
        std::function<void()> onResizeCallback;
        std::function<void(int32_t count, const char **paths)> onDragFilesCallback;
    };

    PlatformWindow(void) = default;
    PlatformWindow(
        const PlatformWindowSettings &settings,
        const std::function<void()> &onResizeCallback = nullptr,
        const std::function<void(int32_t count, const char **paths)> &onDragFilesCallback = nullptr);

    ~PlatformWindow(void);
    
    GLFWwindow *GetGlfwHandle(void) { return m_window; }

    void SetColors(unsigned long titlebar, unsigned long border, unsigned long text);

    const Vector2 GetSize(void) const { return Vector2(m_data.width, m_data.height); }
    const Vector2 GetPosition(void) const { return Vector2(m_data.x, m_data.y); }
    const Vector2 GetFramebufferSize(void) const { return Vector2(m_data.fbWidth, m_data.fbHeight); }
    const Vector2 GetMousePosition(void) const { return Vector2(m_data.mouseX, m_data.mouseY); }
    
    bool IsRunning(void) { return !m_data.shouldClose; }

private:
    GLFWwindow *m_window;
    WindowData m_data;
};

}