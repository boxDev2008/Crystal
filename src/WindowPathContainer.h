#pragma once

#include <filesystem>

namespace Crystal
{

class WindowPathContainer
{
public:
	std::filesystem::path GetFilePath(void) const { return m_filePath; }

protected:
	std::filesystem::path m_filePath;
};

}