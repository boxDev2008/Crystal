#pragma once

#include "Window.h"
#include "Rendering/Renderer.h"
#include "TextEditor.h"

#include "PseudoTerminal.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace Crystal
{

class TerminalWindow : public Window
{
public:
    struct HistoryData
    {
        std::vector<std::string> commandHistroy;
        int32_t historyIndex = -1;
    };

    TerminalWindow(void);
    ~TerminalWindow(void);

    void OnWindowAdded(void);
	void RenderWindow(void);

    void SetCurrentDirectoryPath(const std::filesystem::path &path);
    
private:
    class ProcessPool
    {
    public:
        ProcessPool(void);
        ~ProcessPool(void) { SendCtrlCSignal(); }

        void FetchChildProcesses(unsigned int parentPid) { m_childPids = GetChildProcesses(parentPid); }
        void SendCtrlCSignal(void);

        bool HasChildProcesses(void) { return !m_childPids.empty(); }

    private:
        std::vector<unsigned long> GetChildProcesses(unsigned int parentPid);
        std::vector<unsigned long> m_childPids;
    };
    
    void ExecuteCommand(const std::string &cmd);
    std::string ParseAnsiEscapeCodes(const std::string &input);

    HistoryData m_historyData;

    std::unique_ptr<PseudoTerminal> m_pseudoTerminal;
    std::unique_ptr<ProcessPool> m_processPool;

    ImVec2 m_cursorPosition;

    std::string m_outputBuffer;
    std::string m_lastOutput;

    char m_windowTitle[128];

    bool m_busy = false;
    char m_inputBuffer[256] = "";
};

}