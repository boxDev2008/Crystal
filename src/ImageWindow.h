#pragma once

#include "Window.h"
#include "WindowPathContainer.h"

#include "Math/Vector2.h"
#include "Rendering/Renderer.h"

namespace Crystal
{

class ImageWindow : public Window, public WindowPathContainer
{
	using Vector2 = Math::Vector2;

public:
	ImageWindow(const std::filesystem::path &filePath);
	~ImageWindow(void);

	void SetFilePath(const std::filesystem::path &path) { m_filePath = path; }

	void RenderWindow(void);
	void OnWindowAdded(void);

private:
	float m_zoomFactor = 0.1f;
	float m_smoothZoomFactor = m_zoomFactor;
	Vector2 m_offset;
	Texture *m_texture;
};

}