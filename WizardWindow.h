#pragma once

#include "WindowPlugin.h"
#include "VulkanContext.h"
#include "TextEditor.h"

namespace Crystal
{

class WizardWindow : public WindowPlugin
{
public:
	static std::shared_ptr<WizardWindow> Create(void);
	void RenderWindow(void);
};

}