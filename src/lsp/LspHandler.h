#pragma once

#include <tiny-process/process.hpp>

#include <filesystem>

namespace Crystal
{

class LspHandler
{
public:
    LspHandler(void) = default;
    LspHandler(const std::filesystem::path &workingDirectory);

    ~LspHandler(void);

private:
    std::unique_ptr<TinyProcessLib::Process> m_process;
};

}