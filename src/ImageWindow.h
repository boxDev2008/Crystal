#pragma once

#include "Window.h"
#include "WindowPathContainer.h"
#include "Rendering/Renderer.h"

namespace Crystal
{

class ImageWindow : public Window, public WindowPathContainer
{
public:
	ImageWindow(const std::filesystem::path &filePath);
	~ImageWindow(void);

	void SetFilePath(const std::filesystem::path &path) { m_filePath = path; }

	void RenderWindow(void);
	void OnWindowAdded(void);

private:
	float m_zoomFactor = 0.1f;
	float m_smoothZoomFactor = m_zoomFactor;
	ImVec2 m_offset;
	Texture *m_texture;
};

}