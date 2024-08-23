#include "Resources.h"
#include <filesystem>
#include <unordered_map>

namespace Crystal
{

static std::unordered_map<std::string, Texture*> s_texutres;
static Renderer *s_renderer;

Texture &Resources::GetTextureByName(const std::string &name)
{
	return *s_texutres[name];
}

void Resources::Initialize(Renderer *renderer)
{
	namespace fs = std::filesystem;

	s_renderer = renderer;
    if (!fs::exists("res") || !fs::is_directory("res"))
		return;

    for (const auto &entry : fs::directory_iterator("res"))
	{
		if (!fs::is_regular_file(entry.path()))
			continue;

		fs::path path = entry.path();
		std::string name = path.filename().replace_extension().string();
		std::string fullPath = path.string();
		s_texutres[name] = s_renderer->CreateTexture(fullPath.c_str(), Filter_Linear);
	}
}

void Resources::Shutdown(void)
{
	for (auto &pair : s_texutres)
		s_renderer->DestroyTexture(pair.second);
	s_texutres.clear();
}

}