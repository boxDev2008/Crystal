#pragma once

#include "PlatformWindow.h"

#include "imgui.h"

#include <stdint.h>

namespace Crystal
{

enum Filter
{
	Filter_Nearest = 0,
	Filter_Linear = 1
};

struct Texture
{
    ImTextureID id;
	uint32_t width, height;
};

class Renderer
{
public:
	virtual ~Renderer() = default;

	virtual void BeginFrame(void) = 0;
	virtual void EndFrame(void) = 0;

	virtual Texture *CreateTexture(const char *file, Filter filter = Filter_Nearest) = 0;
	virtual void DestroyTexture(Texture *texture) = 0;

protected:
	PlatformWindow *m_window;
};

}