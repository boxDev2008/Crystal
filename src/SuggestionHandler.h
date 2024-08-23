#pragma once

#include "TextEditor.h"

namespace Crystal
{

struct Suggestion
{
	enum Type
	{
		Keyword,
		Method,
		Class,
		Variable
	};

	Type type;

	std::string word;
	std::string definition;
};

struct SuggestionData
{
	std::vector<std::string> m_initialKeywords = {
		"int", "float", "void", "bool", "unsigned",
		"double", "char", "long", "short", "const",
		"static", "struct", "class", "enum", "namespace",
		"template", "typename", "size_t", "nullptr", "virtual",
		"volatile", "mutable", "friend", "operator", "extern",
		"inline", "typedef", "decltype", "auto", "register",
		"explicit", "private", "protected", "public", "using",
		"throw", "try", "catch", "if", "else",
		"switch", "case", "default", "while", "do",
		"for", "break", "continue", "return", "goto",
		"const_cast", "dynamic_cast", "reinterpret_cast", "static_cast",
		"alignas", "alignof", "char16_t", "char32_t", "constexpr",
		"final", "override", "transaction_safe", "thread_local",
		"asm", "sizeof", "typeof",
		"__declspec", "__attribute__", "__cdecl", "__stdcall",
		"__int8", "__int16", "__int32", "__int64", "__asm__",
		"_Alignas", "_Alignof", "_Atomic", "_Bool", "_Complex",
		"_Generic", "_Imaginary", "_Noreturn", "_Static_assert", "_Thread_local"
	};

	std::vector<std::string> m_initialMethods = {
		"printf", "scanf", "malloc", "free", "calloc", "realloc",
		"strcpy", "strncpy", "strcat", "strncat", "strcmp", "strncmp",
		"strlen", "strchr", "strrchr", "strstr", "strtok", "strerror",
		"memcpy", "memmove", "memcmp", "memset",
		"fprintf", "fscanf", "fgets", "fputs", "fclose", "fopen", "freopen", "feof", "ferror", "fflush", "fputc", "fgetc", "rewind",
		"abs", "fabs", "sqrt", "pow", "ceil", "floor", "round", "rand", "srand",
		"time", "clock", "ctime", "localtime", "gmtime", "strftime", "difftime", "mktime",
		"sin", "cos", "tan", "asin", "acos", "atan", "atan2",
		"sinh", "cosh", "tanh", "asinh", "acosh", "atanh",
		"atoi", "atol", "atof", "itoa", "ltoa", "itoa_s", "itoa_l",
		"cout", "cin", "cerr", "endl", "getline", "push_back", "pop_back", "clear",
		"erase", "insert", "sort", "find", "max", "min"
	};

	std::vector<std::string> m_initialClasses = {
		"std", "vector", "string", "unordered_map", "map", "set",
		"int8_t", "uint8_t", "int16_t", "uint16_t", "int32_t", "uint32_t",
		"int64_t", "uint64_t", "intptr_t", "uintptr_t"
	};

	std::vector<Suggestion> m_additionalSuggestions;
	std::vector<Suggestion> m_filteredSuggestions;
};

class SuggestionHandler
{
public:
    void FilterSuggestions(const std::string& input);
    void InsertSuggestion(TextEditor &editor, const std::string &initialWord, const std::string &suggestion, int wordStart, int wordEnd);
    void RenderSuggestions(TextEditor &editor, const std::string &initialWord, const ImVec2 &windowPosition, bool &focus, int wordStart, int wordEnd);

    bool HasSuggestions(void) const { return !m_suggestionData.m_filteredSuggestions.empty(); }
	SuggestionData &GetSuggestionData(void) { return m_suggestionData; }

private:
    SuggestionData m_suggestionData;
};

}