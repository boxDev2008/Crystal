#pragma once

#include "Utils.h"

#include <string>
#include <functional>
#include <memory>
#include <filesystem>

namespace Crystal
{

class PseudoTerminal
{
protected:
#ifdef CRYSTAL_PLATFORM_WINDOWS
    typedef unsigned long ProcessID;
#elif CRYSTAL_PLATFORM_LINUX
    typedef pid_t ProcessID;
#endif

public:
    static std::unique_ptr<PseudoTerminal> Create(const std::string &shell, const std::filesystem::path &directory);

    virtual ~PseudoTerminal(void) { }

    virtual void Write(const std::string& input) = 0;
    virtual void ReadOutput(const std::function<void(const char*, size_t)>& callback) = 0;

    virtual ProcessID GetPID(void) = 0;
};

}