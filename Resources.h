#pragma once

#include "VulkanContext.h"

#include <iostream>

namespace Crystal
{

class Resources
{
public:
	static VulkanContext::Texture GetTextureByName(std::string name);

	static void Initialize(VulkanContext *vulkanContext);
	static void Shutdown(void);
};

}