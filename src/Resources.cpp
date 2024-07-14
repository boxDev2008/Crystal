#include "Resources.h"
#include <filesystem>
#include <unordered_map>

namespace Crystal
{

static std::unordered_map<std::string, VulkanContext::Texture> s_texutres;
static VulkanContext *s_vkContext;

VulkanContext::Texture Resources::GetTextureByName(std::string name)
{
	return s_texutres[name];
}

void Resources::Initialize(VulkanContext *vkContext)
{
	namespace fs = std::filesystem;

	s_vkContext = vkContext;
    if (!fs::exists("res") || !fs::is_directory("res"))
		return;

    for (const auto &entry : fs::directory_iterator("res"))
	{
		fs::path path = entry.path();
		std::string name = path.filename().replace_extension().string();
		std::string fullPath = path.string();
		s_texutres[name] = s_vkContext->CreateTexture(fullPath.c_str());
	}
}

void Resources::Shutdown(void)
{
	for (auto &pair : s_texutres)
		s_vkContext->DestroyTexture(pair.second);
	s_texutres.clear();
}

}