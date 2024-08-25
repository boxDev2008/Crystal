#include "PlatformWindow.h"
#include "Utils.h"

#include <stb_image.h>

#if CRYSTAL_PLATFORM_WINDOWS
	#include <windows.h>
	#include <dwmapi.h>
	#pragma comment(lib,"dwmapi.lib")

	#define GLFW_EXPOSE_NATIVE_WIN32
	#include <GLFW/glfw3native.h>
#endif

namespace Crystal
{

static uint32_t window_count = 0;

PlatformWindow::PlatformWindow(
	const PlatformWindowSettings &settings,
	const std::function<void()> &onResizeCallback,
	const std::function<void(int32_t count, const char **paths)> &onDragFilesCallback
	)
{
    if (++window_count == 1 && !glfwInit())
        return;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_window = glfwCreateWindow(settings.width, settings.height, settings.title, nullptr, nullptr);

	if (!glfwVulkanSupported())
	{
		printf("GLFW: Vulkan Not Supported\n");
		return;
	}

	glfwSetWindowSizeLimits(m_window, settings.minWidth, settings.minHeight, GLFW_DONT_CARE, GLFW_DONT_CARE);

    if (settings.iconPath)
    {
        GLFWimage icon; 
        icon.pixels = stbi_load(settings.iconPath, &icon.width, &icon.height, 0, 4);
        glfwSetWindowIcon(m_window, 1, &icon); 
        stbi_image_free(icon.pixels);
    }

	m_data.onResizeCallback = onResizeCallback;
	m_data.onDragFilesCallback = onDragFilesCallback;

	glfwGetWindowPos(m_window, &m_data.x, &m_data.y);
	glfwGetWindowSize(m_window, &m_data.width, &m_data.height);

	glfwSetWindowUserPointer(m_window, &m_data);

	glfwSetWindowPosCallback(m_window, [](GLFWwindow* window, int32_t x, int32_t y)
	{
		WindowData &data = *(WindowData*)glfwGetWindowUserPointer(window);
		data.x = x;
		data.y = y;
		data.moving = true;
	});

	glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int32_t width, int32_t height)
	{
		WindowData &data = *(WindowData*)glfwGetWindowUserPointer(window);
		data.width = width;
		data.height = height;
		data.resizing = true;
	});

	glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* window, int32_t width, int32_t height)
	{
		WindowData &data = *(WindowData*)glfwGetWindowUserPointer(window);
		data.fbWidth = width;
		data.fbHeight = height;
		data.onResizeCallback();
	});

	glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xPos, double yPos)
	{
		WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        data.mouseX = xPos;
        data.mouseY = yPos;
	});

	glfwSetDropCallback(m_window, [](GLFWwindow* window, int32_t count, const char **paths)
	{
		WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
		data.onDragFilesCallback(count, paths);
	});

	glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window)
	{
		WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        data.shouldClose = true;
	});
}

PlatformWindow::~PlatformWindow(void)
{
    glfwDestroyWindow(m_window);

    if (--window_count == 0)
        glfwTerminate();
}

void PlatformWindow::PollEvents(void)
{
	m_data.resizing = false;
	m_data.moving = false;
	glfwPollEvents();
}

void PlatformWindow::SetColors(unsigned long titlebar, unsigned long border, unsigned long text)
{
#if CRYSTAL_PLATFORM_WINDOWS
    HWND win32Handle = glfwGetWin32Window(m_window);

    DwmSetWindowAttribute(
        win32Handle, 34,
        &border, sizeof(border)
    );

    DwmSetWindowAttribute(
        win32Handle, 35,
        &titlebar, sizeof(titlebar)
    );

    DwmSetWindowAttribute(
        win32Handle, 36,
        &text, sizeof(text)
    );
#endif
}

}