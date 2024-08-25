#pragma once

#include "Rendering/Renderer.h"

#include <iostream>

namespace Crystal
{

class Resources
{
public:
	static Texture &GetTextureByName(const std::string &name);

	static void Initialize(Renderer *renderer);
	static void Shutdown(void);
};

}