#include "PseudoTerminal.h"
#include "Utils.h"

#if CRYSTAL_PLATFORM_LINUX

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <thread>
#include <atomic>
#include <cstring>
#include <functional>
#include <iostream>

namespace Crystal
{

class LinuxPseudoTerminal : public PseudoTerminal
{
public:
    LinuxPseudoTerminal(const std::string &shell, const std::filesystem::path &directory)
        : masterFd(-1), slaveFd(-1), childPid(-1), m_readClosed(false)
    {
        CreatePseudoTerminal();
        StartChildProcess(shell, directory);
    }

    ~LinuxPseudoTerminal(void)
    {
        Cleanup();
    }

    void Write(const std::string& input) override
    {
        if (write(masterFd, input.c_str(), input.size()) == -1)
            ErrorExit("Failed to write to pseudo terminal.");
    }

    void ReadOutput(const std::function<void(const char*, size_t)>& callback)
    {
        m_readThread = std::thread([this, callback]()
        {
            char buffer[1024];
            while (!m_readClosed)
            {
                ssize_t bytesRead = read(masterFd, buffer, sizeof(buffer) - 1);
                if (bytesRead > 0)
                {
                    callback(buffer, bytesRead);
                }
                else if (bytesRead == -1 && errno != EAGAIN && errno != EINTR)
                {
                    break;  // An error occurred
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
    }

private:
    int masterFd, slaveFd;
    pid_t childPid;
    std::thread m_readThread;
    std::atomic<bool> m_readClosed;

    void CreatePseudoTerminal(void)
    {
        masterFd = posix_openpt(O_RDWR | O_NOCTTY);
        if (masterFd == -1)
            ErrorExit("Failed to open pseudo terminal master.");

        if (grantpt(masterFd) == -1 || unlockpt(masterFd) == -1)
            ErrorExit("Failed to grant or unlock pseudo terminal.");

        slaveFd = open(ptsname(masterFd), O_RDWR | O_NOCTTY);
        if (slaveFd == -1)
            ErrorExit("Failed to open pseudo terminal slave.");
    }

    void StartChildProcess(const std::string &shell, const std::filesystem::path &directory)
    {
        childPid = fork();
        if (childPid == -1)
        {
            ErrorExit("Failed to fork process.");
        }
        else if (childPid == 0)
        {
            // Child process
            if (setsid() == -1)
                ErrorExit("Failed to create new session.");

            if (dup2(slaveFd, STDIN_FILENO) == -1 ||
                dup2(slaveFd, STDOUT_FILENO) == -1 ||
                dup2(slaveFd, STDERR_FILENO) == -1)
                ErrorExit("Failed to duplicate file descriptors.");

            close(masterFd);
            close(slaveFd);

            if (!directory.empty())
            {
                if (chdir(directory.c_str()) == -1)
                    ErrorExit("Failed to change directory.");
            }

            execlp(shell.c_str(), shell.c_str(), nullptr);
            ErrorExit("Failed to execute shell.");
        }
        else
        {
            // Parent process
            close(slaveFd);
        }
    }

    void ErrorExit(const char* message)
    {
        std::cerr << message << ": " << strerror(errno) << '\n';
        exit(EXIT_FAILURE);
    }

    void Cleanup(void)
    {
        m_readClosed = true;

        if (m_readThread.joinable())
            m_readThread.join();

        if (childPid != -1)
        {
            kill(childPid, SIGKILL);
            waitpid(childPid, nullptr, 0);
        }

        if (masterFd != -1)
            close(masterFd);
    }

    PseudoTerminal::ProcessID GetPID(void) override { return childPid; }
};

}

#endif