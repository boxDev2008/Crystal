#pragma once

#include "WindowManager.h"
#include "PlatformWindow.h"
#include "Preferences.h"
#include "rendering/Renderer.h"

#include "BuildErrorHandler.h"
#include "SuggestionHandler.h"
#include "LayoutHandler.h"
#include "DragDropHandler.h"

namespace Crystal
{

class Application
{
public:
    Application(void);
	void OnRender(void);

	void OpenFile(const std::filesystem::path &path);

	PlatformWindow &GetMainWindow(void) { return *m_mainWindow; }
	Renderer &GetRenderer(void) { return *m_renderer; }
	WindowManager &GetWindowManager(void) { return m_windowManager; }
	BuildErrorHandler &GetBuildErrorHandler(void) { return m_buildErrorHandler; }
	//SuggestionHandler &GetSuggestionHandler(void) { return m_suggestionHandler; }
	DragDropHandler &GetDragDropHandler(void) { return m_dragDropHandler; }
	LayoutHandler &GetLayoutHandler(void) { return m_layoutHandler; }

	Preferences &GetPreferences(void) { return m_preferences; }

	void SetMainDirectoryPath(const std::filesystem::path &path);
	std::filesystem::path GetMainDirectoryPath(void) const { return m_mainDirectory; }

private:
	std::filesystem::path m_mainDirectory;

	BuildErrorHandler m_buildErrorHandler;
	//SuggestionHandler m_suggestionHandler;
	DragDropHandler m_dragDropHandler;
	LayoutHandler m_layoutHandler;
	Preferences m_preferences;

	//std::unique_ptr<LspHandler> m_lspHandler;

	Renderer *m_renderer;
	WindowManager m_windowManager;
	PlatformWindow *m_mainWindow;
};

}