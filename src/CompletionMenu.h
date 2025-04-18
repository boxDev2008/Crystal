#pragma once

#include "EditorWindow.h"
#include "Application.h"

#include <unordered_set>
#include <string>
#include <cstdint>

namespace Crystal
{
class CompletionMenu
{
public:
    static void SetCurrentWord(const std::string &word, int32_t wordStart, int32_t wordEnd);

    static void ClearCompletions(void);
    static void AddCompletion(const std::string &completion);
    static void RenderCompletionMenu(EditorWindow *parentWindow, Application *application);

private:
    static bool FilterCompletions(void);
};
}
