#include "Win32PseudoTerminal.h"
#include "LinuxPseudoTerminal.h"

namespace Crystal
{

std::unique_ptr<PseudoTerminal> PseudoTerminal::Create(const std::string &shell, const std::filesystem::path &directory)
{
#if CRYSTAL_PLATFORM_WINDOWS
    return std::make_unique<Win32PseudoTerminal>(shell, directory);
#elif CRYSTAL_PLATFORM_LINUX
    return std::make_unique<LinuxPseudoTerminal>(shell, directory);
#endif
}

}