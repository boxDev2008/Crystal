#include "Utils.h"
#include <iostream>

namespace Crystal {
namespace Utils
{

bool isDelimiter(char c)
{
    static const std::string delimiters = " \t!@#$%^&*()+=-{}[];:'\",.<>/?|\\";
    return (std::isspace(c) || delimiters.find(c) != std::string::npos);
}

}
}