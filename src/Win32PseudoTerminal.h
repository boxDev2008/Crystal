#pragma once

#include "PseudoTerminal.h"
#include "Utils.h"

#if CRYSTAL_PLATFORM_WINDOWS

#include <windows.h>
#include <thread>
#include <atomic>

namespace Crystal
{

class Win32PseudoTerminal : public PseudoTerminal
{
public:
    Win32PseudoTerminal(const std::string &shell, const std::filesystem::path &directory) : hPipeInRead(nullptr), hPipeInWrite(nullptr),
                       hPipeOutRead(nullptr), hPipeOutWrite(nullptr),
                       hPseudoConsole(nullptr), pi{ 0 }, siEx{ 0 }
    {
        CreatePipes();
        CreatePseudoTerminal();
        StartChildProcess(shell, directory);
    }

    ~Win32PseudoTerminal(void)
    {
        Cleanup();
    }

    void Write(const std::string& input) override
    {
        DWORD bytesWritten;
        if (!WriteFile(hPipeInWrite, input.c_str(), input.length(), &bytesWritten, NULL))
            ErrorExit("Failed to write to console.");
    }

    void ReadOutput(const std::function<void(const char*, size_t)>& callback) 
    {
        m_readThread = std::thread([this, callback]()
        {
            char buffer[1024];
            DWORD bytesRead;

            while (!m_readClosed)
            {
                DWORD bytesAvailable = 0;
                if (PeekNamedPipe(hPipeOutRead, NULL, 0, NULL, &bytesAvailable, NULL) && bytesAvailable > 0)
                    if (ReadFile(hPipeOutRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL))
                        if (bytesRead > 0) callback(buffer, bytesRead);
                std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Used to patch cpu usage issue
            }
        });
    }

private:
    HANDLE hPipeInRead, hPipeInWrite;
    HANDLE hPipeOutRead, hPipeOutWrite;
    HANDLE hPseudoConsole;
    PROCESS_INFORMATION pi;
    STARTUPINFOEXA siEx;
    std::thread m_readThread;
    std::atomic<bool> m_readClosed = false;

    void CreatePipes(void)
    {
        SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

        if (!CreatePipe(&hPipeInRead, &hPipeInWrite, &sa, 0))
            ErrorExit("Failed to create input pipe.");

        if (!CreatePipe(&hPipeOutRead, &hPipeOutWrite, &sa, 0))
            ErrorExit("Failed to create output pipe.");

        if (!SetHandleInformation(hPipeInWrite, HANDLE_FLAG_INHERIT, 0))
            ErrorExit("Failed to set handle information for input pipe.");

        if (!SetHandleInformation(hPipeOutRead, HANDLE_FLAG_INHERIT, 0))
            ErrorExit("Failed to set handle information for output pipe.");
    }

    void CreatePseudoTerminal(void)
    {
        COORD consoleSize = { 1024, 2 };
        HRESULT hr = CreatePseudoConsole(consoleSize, hPipeInRead, hPipeOutWrite, 0, &hPseudoConsole);
        if (FAILED(hr))
            ErrorExit("Failed to create pseudo console.");
    }

    void StartChildProcess(const std::string &shell, const std::filesystem::path &directory)
    {
        SIZE_T attrListSize;

        ZeroMemory(&siEx, sizeof(STARTUPINFOEXA));
        siEx.StartupInfo.cb = sizeof(STARTUPINFOEXA);

        InitializeProcThreadAttributeList(NULL, 1, 0, &attrListSize);

        siEx.lpAttributeList = (LPPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(GetProcessHeap(), 0, attrListSize);
        if (!siEx.lpAttributeList)
            ErrorExit("HeapAlloc failed for attribute list.");

        if (!InitializeProcThreadAttributeList(siEx.lpAttributeList, 1, 0, &attrListSize))
            ErrorExit("Failed to initialize the attribute list.");

        if (!UpdateProcThreadAttribute(siEx.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE, hPseudoConsole, sizeof(HANDLE), NULL, NULL))
            ErrorExit("Failed to set pseudo console attribute.");

        BOOL result = CreateProcessA(
            NULL,
            (LPSTR)shell.c_str(),
            NULL,
            NULL,
            TRUE,
            EXTENDED_STARTUPINFO_PRESENT,
            NULL,
            directory.empty() ? NULL : directory.string().data(),
            &siEx.StartupInfo,
            &pi
        );

        if (!result)
            ErrorExit("Failed to create process.");
    }

    void ErrorExit(const char* message)
    {
        std::cerr << message << " Error: " << GetLastError() << '\n';
        ExitProcess(0);
    }

    void Cleanup(void)
    {
        m_readClosed = true;

        if (m_readThread.joinable())
            m_readThread.join();

        if (pi.hProcess) CloseHandle(pi.hProcess);
        if (pi.hThread) CloseHandle(pi.hThread);
        if (hPseudoConsole) CloseHandle(hPseudoConsole);
        if (hPipeInRead) CloseHandle(hPipeInRead);
        if (hPipeInWrite) CloseHandle(hPipeInWrite);
        if (hPipeOutRead) CloseHandle(hPipeOutRead);
        if (hPipeOutWrite) CloseHandle(hPipeOutWrite);

        if (siEx.lpAttributeList)
        {
            DeleteProcThreadAttributeList(siEx.lpAttributeList);
            HeapFree(GetProcessHeap(), 0, siEx.lpAttributeList);
        }
    }

    PseudoTerminal::ProcessID GetPID(void) override { return pi.dwProcessId; }
};

}

#endif