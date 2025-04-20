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

static uint32_t s_windowCount = 0;

#if CRYSTAL_PLATFORM_WINDOWS

static WNDPROC s_originalProc;
static int32_t s_captionHeight = 0;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_NCCALCSIZE:
        {
            if (!wParam)
                break;

            NCCALCSIZE_PARAMS *params = (NCCALCSIZE_PARAMS*)lParam;
            RECT *requestedClientRect = params->rgrc;

            const int32_t resizeBorderX = GetSystemMetrics(SM_CXFRAME);
            const int32_t resizeBorderY = GetSystemMetrics(SM_CYFRAME);

            requestedClientRect->right -= resizeBorderX;
            requestedClientRect->left += resizeBorderX;
            requestedClientRect->bottom -= resizeBorderY;
            requestedClientRect->top += IsZoomed(hWnd) ? resizeBorderY : 0;

            return WVR_ALIGNTOP | WVR_ALIGNLEFT;
        }
        case WM_NCPAINT:
        {
            break;
        }
        case WM_NCHITTEST:
        {
            POINTS mousePos = MAKEPOINTS(lParam);
            POINT screenMousePos = { mousePos.x, mousePos.y };

            RECT windowRect;
            GetWindowRect(hWnd, &windowRect);

            const int borderWidth = 8;

            int relativeX = screenMousePos.x - windowRect.left;
            int relativeY = screenMousePos.y - windowRect.top;

            if (relativeY >= (windowRect.bottom - windowRect.top) - borderWidth)
                if (relativeX <= borderWidth)
                    return HTBOTTOMLEFT;
                else if (relativeX >= (windowRect.right - windowRect.left) - borderWidth)
                    return HTBOTTOMRIGHT;
                else
                    return HTBOTTOM;
            else if (relativeY <= borderWidth)
                if (relativeX <= borderWidth)
                    return HTTOPLEFT;
                else if (relativeX >= (windowRect.right - windowRect.left) - borderWidth)
                    return HTTOPRIGHT;
                else
                    return HTTOP;
            else if (relativeX <= borderWidth)
                return HTLEFT;
            else if (relativeX >= (windowRect.right - windowRect.left) - borderWidth)
                return HTRIGHT;
            else if (relativeY <= s_captionHeight)
                return HTCAPTION;

            break;
        }
        case WM_NCACTIVATE:
            break;
    }

    return CallWindowProc(s_originalProc, hWnd, uMsg, wParam, lParam);
}

void PlatformWindow::SetCaptionHeight(int32_t value)
{
	s_captionHeight = value;
}

#endif

PlatformWindow::PlatformWindow(
	const PlatformWindowSettings &settings,
	const std::function<void()> &onForceRedrawCallback,
	const std::function<void(int32_t count, const char **paths)> &onDragFilesCallback
	)
{
    if (++s_windowCount == 1 && !glfwInit())
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

	m_data.onForceRedrawCallback = onForceRedrawCallback;
	m_data.onDragFilesCallback = onDragFilesCallback;

	glfwGetWindowPos(m_window, &m_data.x, &m_data.y);
	glfwGetWindowSize(m_window, &m_data.width, &m_data.height);

#if CRYSTAL_PLATFORM_WINDOWS
    HWND hWnd = glfwGetWin32Window(m_window);

    LONG_PTR lStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
    lStyle |= WS_THICKFRAME | WS_CAPTION;
    SetWindowLongPtr(hWnd, GWL_STYLE, lStyle);

    //DWMNCRENDERINGPOLICY policy = DWMNCRP_ENABLED;
    //DwmSetWindowAttribute(hWnd, DWMWA_NCRENDERING_POLICY, &policy, sizeof(policy));

    //MARGINS margins = { -1 };
    //DwmExtendFrameIntoClientArea(hWnd, &margins);

    RECT windowRect;
    GetWindowRect(hWnd, &windowRect);
    int width = windowRect.right - windowRect.left;
    int height = windowRect.bottom - windowRect.top;

    s_originalProc = (WNDPROC)GetWindowLongPtr(hWnd, GWLP_WNDPROC);
    SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProc));
    SetWindowPos(hWnd, NULL, 0, 0, width, height, SWP_FRAMECHANGED | SWP_NOMOVE);
#endif

	glfwSetWindowUserPointer(m_window, &m_data);

	glfwSetWindowPosCallback(m_window, [](GLFWwindow* window, int32_t x, int32_t y)
	{
		WindowData &data = *(WindowData*)glfwGetWindowUserPointer(window);
		data.x = x;
		data.y = y;
        data.moving = true;
        data.onForceRedrawCallback();
	});

	glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int32_t width, int32_t height)
	{
		WindowData &data = *(WindowData*)glfwGetWindowUserPointer(window);
		data.width = width;
		data.height = height;
	});

	glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* window, int32_t width, int32_t height)
	{
		WindowData &data = *(WindowData*)glfwGetWindowUserPointer(window);
		data.fbWidth = width;
		data.fbHeight = height;
        data.resizing = true;
        data.onForceRedrawCallback();
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

    if (--s_windowCount == 0)
        glfwTerminate();
}

void PlatformWindow::PollEvents(void)
{
	if (m_data.maxmizing)
	{
		m_data.resizing = false;
		m_data.moving = false;
	}
	else
	{
		m_data.resizing = true;
		m_data.maxmizing = true;
	}

	glfwPollEvents();
}

void PlatformWindow::SetBorderColor(unsigned long color)
{
#if CRYSTAL_PLATFORM_WINDOWS
    HWND hwnd = glfwGetWin32Window(m_window);

    DwmSetWindowAttribute(
        hwnd, 34,
        &color, sizeof(color)
    );

    /*DwmSetWindowAttribute(
        hwnd, 35,
        &titlebar, sizeof(titlebar)
    );

    DwmSetWindowAttribute(
        hwnd, 36,
        &text, sizeof(text)
    );*/
#endif
}

}