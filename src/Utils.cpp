#include "Utils.h"
#include <algorithm>
#include <regex>

namespace Crystal {
namespace Utils
{

bool isDelimiter(char c)
{
    static const std::string delimiters = " \t!@#$%^&*()+=-{}[];:'\",.<>/?|\\";
    return (std::isspace(c) || delimiters.find(c) != std::string::npos);
}

bool isWhitespace(char c)
{
    return std::isspace(static_cast<unsigned char>(c));
}

bool isBoundaryChar(char c)
{
    return isspace(c) || ispunct(c) || c == '(' || c == ')' || c == '{' || c == '}' ||
        c == '[' || c == ']' || c == '<' || c == '>' || c == ';' || c == ',' && c != '_';
}

std::string toLower(const std::string &str)
{
    std::string lower_str = str;
    std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), ::tolower);
    return lower_str;
}

std::string getLastLine(const std::string &text)
{
	size_t pos = text.find_last_of("\n");
	if (pos == std::string::npos)
		return text;
	else
		return text.substr(pos + 1);
}

}
}