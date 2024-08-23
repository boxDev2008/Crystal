#pragma once

#include <iostream>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define CRYSTAL_PLATFORM_WINDOWS 1
#elif defined(__linux__) || defined(__gnu_linux__)
#define CRYSTAL_PLATFORM_LINUX 1
#endif

namespace Crystal
{

namespace Utils
{

bool isDelimiter(char c);
bool isWhitespace(char c);
bool isBoundaryChar(char c);

std::string toLower(const std::string &str);
std::string getLastLine(const std::string &text);

}
}