#pragma once

#include <unordered_map>

namespace Crystal
{

class IntellisenseHandler
{
public:
    IntellisenseHandler(void);
    ~IntellisenseHandler(void);

private:
    void ExtractCompletions(void);

	std::unordered_map<std::string, std::vector<std::string>> m_functions;
};

}