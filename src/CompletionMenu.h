#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>

namespace Crystal
{

enum CompletionType
{
	COMPLETION_TYPE_FUNCTION,
	COMPLETION_TYPE_VARIABLE,
	COMPLETION_TYPE_TYPE,
	COMPLETION_TYPE_NAMESPACE
};

class Application;
class EditorWindow;
class CompletionMenu
{
public:
	CompletionMenu(void) = default;
	CompletionMenu(EditorWindow *parentWindow, Application *application);
    void SetCurrentWord(const std::string &word, int32_t wordStart, int32_t wordEnd);

    void ClearCompletions(void);
    void AddCompletion(const std::string &completion, CompletionType type);
    void Render(void);

private:
    bool FilterCompletions(void);

	bool m_active = false;

    int32_t m_wordStart, m_wordEnd;
    std::string m_word;

    std::unordered_map<std::string, CompletionType> m_completions{};
    std::unordered_map<std::string, CompletionType> m_filteredCompletions{};

	EditorWindow *m_parentWindow;
	Application *m_application;
};
}
