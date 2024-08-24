#include "TerminalWindow.h"
#include "Application.h"
#include "Utils.h"
#include <algorithm>
#include <cmath>

#include "imgui_internal.h"

#include <stdio.h>

#include <codecvt>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <unordered_map>

#if CRYSTAL_PLATFORM_WINDOWS
    #include <windows.h>
    #include <TlHelp32.h>
#elif CRYSTAL_PLATFORM_LINUX
    #include <sys/types.h>
    #include <dirent.h>
    #include <unistd.h>
    #include <signal.h>
#endif

namespace Crystal
{

#if CRYSTAL_PLATFORM_WINDOWS

TerminalWindow::ProcessPool::ProcessPool(void)
{
    SetConsoleCtrlHandler(NULL, FALSE);
}

void TerminalWindow::ProcessPool::SendCtrlCSignal(void)
{
    FreeConsole();

    for (unsigned int pid : m_childPids)
    {
        if (!AttachConsole(pid))
        {
            std::cerr << "Fail to attach to the console of process " << pid << std::endl;
            continue;
        }

        SetConsoleCtrlHandler(NULL, TRUE);

        if (!GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0))
            std::cerr << "Fail to generate console event for process " << pid << std::endl;

        FreeConsole();
    }

    if (!AttachConsole(ATTACH_PARENT_PROCESS))
        std::cerr << "Failed to reattach the console to the parent process." << std::endl;
}

std::vector<unsigned long> TerminalWindow::ProcessPool::GetChildProcesses(unsigned int parentPid)
{
    std::vector<DWORD> childPids;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot == INVALID_HANDLE_VALUE)
    {
        std::cerr << "Failed to create process snapshot." << std::endl;
        return childPids;
    }

    PROCESSENTRY32 processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(snapshot, &processEntry))
    {
        do
        {
            if (processEntry.th32ParentProcessID == parentPid && _wcsicmp(processEntry.szExeFile, L"conhost.exe"))
                childPids.push_back(processEntry.th32ProcessID);
        }
        while (Process32Next(snapshot, &processEntry));
    }
    else std::cerr << "Failed to retrieve process information." << std::endl;

    CloseHandle(snapshot);
    return childPids;
}

#elif CRYSTAL_PLATFORM_LINUX

TerminalWindow::ProcessPool::ProcessPool(void) = default;

void TerminalWindow::ProcessPool::SendCtrlCSignal(void)
{
    for (unsigned long pid : m_childPids)
        if (kill(pid, SIGINT) != 0)
            std::cerr << "Failed to send SIGINT to process " << pid << ": " << strerror(errno) << std::endl;
}

std::vector<unsigned long> TerminalWindow::ProcessPool::GetChildProcesses(unsigned int parentPid)
{
    std::vector<unsigned long> childPids;
    DIR* dir = opendir("/proc");

    if (!dir)
    {
        std::cerr << "Failed to open /proc directory." << std::endl;
        return childPids;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr)
    {
        if (entry->d_type == DT_DIR)
        {
            unsigned long pid = strtoul(entry->d_name, nullptr, 10);
            if (pid > 0)
            {
                std::string statusPath = std::string("/proc/") + entry->d_name + "/status";
                FILE* statusFile = fopen(statusPath.c_str(), "r");

                if (statusFile)
                {
                    char line[256];
                    while (fgets(line, sizeof(line), statusFile))
                    {
                        if (strncmp(line, "PPid:", 5) == 0)
                        {
                            unsigned long ppid = strtoul(line + 5, nullptr, 10);
                            if (ppid == parentPid)
                                childPids.push_back(pid);
                            break;
                        }
                    }
                    fclose(statusFile);
                }
            }
        }
    }
    closedir(dir);
    return childPids;
}

#endif


TerminalWindow::TerminalWindow(void)
{
    
}

TerminalWindow::~TerminalWindow(void)
{
    //m_process->close_stdin();
    //m_process->kill(true);
}

void TerminalWindow::OnWindowAdded(void)
{
    auto path = m_application->GetMainDirectoryPath();
    SetCurrentDirectoryPath(path.string());
}

std::string TerminalWindow::ParseAnsiEscapeCodes(const std::string &input)
{
    std::string result;
    std::string formatedString;
    size_t i = 0;
    while (i < input.size())
    {
        if (input[i] == '\x1B')
        {
            i++;
            if (i < input.size() && input[i] == '[')
            {
                i++;
                std::string parameters;
                while (i < input.size() && !isalpha(input[i]))
                {
                    parameters += input[i];
                    i++;
                }
                if (i < input.size())
                {
                    char command = input[i];
                    i++;
                    switch (command)
                    {
                        case 'm':
                        {
                            std::cout << "Detected color sequence: " << parameters << '\n';
                            break;
                        }
                        case 'H':
                        {
                            break;
                        }
                        case 'K':
                        {
                            result.erase(std::remove(result.begin(), result.end(), '\n'), result.cend());
                            break;
                        }
                        case 'J':
                        {
                            switch (parameters[0])
                            {
                            case '0':
                                std::cout << "Detected 'erase from cursor until end of screen' command.\n";
                                break;
                            case '1':
                                std::cout << "Detected 'erase from cursor to beginning of screen' command.\n";
                                break;
                            case '2':
                                std::cout << "Detected 'erase entire screen' command.\n";
                                m_outputBuffer.clear();
                                break;
                            case '3':
                                std::cout << "Detected 'erase saved lines' command.\n";
                                m_outputBuffer.clear();
                                break;
                            }
                            break;
                        }
                        default:
                            break;
                    }
                }
            }
            else if (i < input.size() && input[i] == ']')
            {
                i++;
                std::string sequenceType;
                while (i < input.size() && input[i] != ';')
                {
                    sequenceType += input[i];
                    i++;
                }
                if (i < input.size())
                {
                    i++;
                    std::string content;
                    while (i < input.size() && input[i] != '\x07' && input[i] != '\x1B')
                    {
                        content += input[i];
                        i++;
                    }
                    std::cout << "Detected OSC sequence: " << sequenceType << "; " << content << std::endl;
                    if (i < input.size() && input[i] == '\x07')
                        i++;
                }
            }
        }
        else
        {
            result += input[i];
            i++;
        }
    }

    return result;
}

void TerminalWindow::SetCurrentDirectoryPath(const std::filesystem::path &path)
{
    /*if (m_process != nullptr)
    {
        m_process->close_stdin();
        m_process->kill(true);
    }*/

    m_outputBuffer.clear();

    m_processPool = std::make_unique<ProcessPool>();

#ifdef CRYSTAL_PLATFORM_WINDOWS
    m_pseudoTerminal = PseudoTerminal::Create("cmd.exe", path);
#elif CRYSTAL_PLATFORM_LINUX
    m_pseudoTerminal = PseudoTerminal::Create("bash", path);
#endif

    m_pseudoTerminal->ReadOutput([this](const char *bytes, size_t size) {
        std::string buffer = std::string(bytes, size);
        //auto formatedBuffer = ParseAnsiEscapeCodesColored(buffer);
        buffer = ParseAnsiEscapeCodes(buffer);
        m_outputBuffer += buffer;
        m_lastOutput += buffer;
        m_processPool->FetchChildProcesses(m_pseudoTerminal->GetPID());
        m_busy = m_processPool->HasChildProcesses();

        if (!m_busy && m_lastOutput.find("MSBuild") != std::string::npos || m_lastOutput.find("msbuild") != std::string::npos)
            m_application->GetBuildErrorHandler().ApplyErrorMarkersToAllEditorWindows(m_application->GetWindowManager(),
            m_lastOutput, BuildErrorHandler::CompilerType::MSBuild);
    });

    /*m_process = std::make_unique<TinyProcessLib::Process>(
        L"cmd.exe", path.wstring(),
        [this](const char *bytes, size_t n) {
            static int32_t writeIndex = 2;
            for (size_t i = 0; i < n; i++)
            {
                auto &byte = bytes[i];
                if (byte == 12)
                {
                    writeIndex = 0;
                    m_outputBuffer.clear();
                }
                else
                {
                    if (writeIndex > 1)
                    {
                        m_outputBuffer += bytes[i];
                        m_lastOutput += bytes[i];
                    }
                    writeIndex++;
                }
            }
            m_processPool->FetchChildProcesses(m_process->get_id());
        },
        [this](const char *bytes, size_t n) {
            for (size_t i = 0; i < n; i++)
            {
                auto &byte = bytes[i];
                if (byte == 12)
                    m_outputBuffer.clear();
                else
                {
                    m_lastOutput += bytes[i];
                    m_outputBuffer += bytes[i];
                }
            }
            m_processPool->FetchChildProcesses(m_process->get_id());
        },
        true);*/
}

void TerminalWindow::RenderWindow(void)
{
    using Vector2 = Crystal::Math::Vector2;

    static auto isAnyKeyDown = []() -> bool
    {
        ImGuiIO &io = ImGui::GetIO();
        for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); i++)
            if (io.KeysDown[i])
                return true;
        return false;
    };

    static auto inputTextCallback = [](ImGuiInputTextCallbackData* gyatt) -> int32_t
    {
        HistoryData *history = (HistoryData*)gyatt->UserData;
        auto& commands = history->commandHistroy;

        auto updateBuffer = [&](int32_t index) -> void
        {
            gyatt->BufDirty = true;
            gyatt->BufTextLen = commands[commands.size() - 1 - index].size();
            gyatt->CursorPos = gyatt->BufTextLen;
            strcpy(gyatt->Buf, commands[commands.size() - 1 - index].c_str());
        };

        auto clearBuffer = [&](void) -> void
        {
            gyatt->BufDirty = true;
            gyatt->BufTextLen = 0;
            gyatt->CursorPos = 0;
            memset(gyatt->Buf, 0, strlen(gyatt->Buf));
        };

        switch (gyatt->EventKey)
        {
            case ImGuiKey_DownArrow:
                if (history->historyIndex > 0)
                {
                    history->historyIndex--;
                    updateBuffer(history->historyIndex);
                }
                else if (history->historyIndex == 0)
                {
                    history->historyIndex--;
                    clearBuffer();
                }
                break;
            case ImGuiKey_UpArrow:
                if (history->historyIndex < (int32_t)commands.size() - 1)
                {
                    history->historyIndex++;
                    updateBuffer(history->historyIndex);
                }
                break;
        }

        return 0;
    };

	ImGui::SetNextWindowDockID(m_application->GetLayoutHandler().GetBottomDockID(), ImGuiCond_Appearing);

    std::string title = "Terminal - cmd.exe ##" + std::to_string((uintptr_t)this);
    if (ImGui::Begin(title.c_str(), &m_opened, ImGuiWindowFlags_NoNav))
    {
        ImGuiStyle &style = ImGui::GetStyle();
        ImGui::PushStyleColor(ImGuiCol_ChildBg, {0.0f, 0.0f, 0.0f, 0.0f});
        ImGui::PushStyleColor(ImGuiCol_FrameBg, {0.0f, 0.0f, 0.0f, 0.0f});
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, {0.0f, 0.0f, 0.0f, 0.0f});
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, {0.0f, 0.0f, 0.0f, 0.0f});
        ImGui::PushStyleColor(ImGuiCol_NavHighlight, {0.0f, 0.0f, 0.0f, 0.0f});

        if (ImGui::BeginChild("##ScrollingRegion", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), 0, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_NoNav))
        {
            const bool isFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
            const bool isHovered = ImGui::IsWindowHovered(ImGuiFocusedFlags_RootAndChildWindows);

            if (isFocused && ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_C, false))
                m_processPool->SendCtrlCSignal();

            ImGui::TextWrapped("%s", m_outputBuffer.c_str());

            if (!m_busy)
            {
                const ImU32 writingColor = m_application->GetPreferences().GetGlobalSettings().GetEditorPalette()[1];
                ImGui::PushStyleColor(ImGuiCol_Text, writingColor);

                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::CalcTextSize(Utils::getLastLine(m_outputBuffer).c_str()).x - style.FramePadding.x);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetFontSize() - style.FramePadding.y * 1.5f);

                if (ImGui::BeginChild("##InputChild", ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 1.5), 0, ImGuiWindowFlags_NoNav))
                {
                    ImGui::PushItemWidth(-1); 

                    if ((isFocused && isAnyKeyDown()) || (isHovered && ImGui::IsAnyMouseDown()))
                        ImGui::SetKeyboardFocusHere();
                    if (ImGui::InputText("##Input", m_inputBuffer, 256,
                        ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_NoHorizontalScroll |
                        ImGuiInputTextFlags_CallbackHistory, inputTextCallback, &m_historyData))
                    {
                        std::string command(m_inputBuffer);

                        if (!command.empty())
                        {
                            if (m_historyData.commandHistroy.empty())
                                m_historyData.commandHistroy.push_back(command);
                            else if (m_historyData.commandHistroy[m_historyData.commandHistroy.size() - 1] != command)
                                m_historyData.commandHistroy.push_back(command);

                            m_lastOutput.clear();
                            m_pseudoTerminal->Write(command + "\r\n");

                            m_historyData.historyIndex = -1;
                            m_inputBuffer[0] = '\0';
                        }
                        ImGui::SetKeyboardFocusHere(-1);
                    }

                    ImGui::PopItemWidth();
                }
                ImGui::EndChild();
                ImGui::PopStyleColor();
            }

            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndChild();
        ImGui::PopStyleColor(5);
    }
    ImGui::End();
}

}