#include "LspHandler.h"
#include <iostream>

#include <nlohmann/json.hpp>

namespace Crystal
{
static int i = 0;

LspHandler::LspHandler(const std::filesystem::path &workingDirectory)
{
    m_process = std::make_unique<TinyProcessLib::Process>(
        L"cmd.exe /C clangd.exe --log=verbose", L"",
        [this](const char *bytes, size_t n) {
            std::string buffer = std::string(bytes, n);
            std::cout << buffer << '\n';
            i++;
        },
        [this](const char *bytes, size_t n) {
            std::string buffer = std::string(bytes, n);
            std::cout << buffer << '\n';
            i++;
        },
        true);
    
    nlohmann::json initialize_params = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "initialize"},
        {"params", {
            {"processId", nullptr},
            {"rootUri", "file:///" + workingDirectory.string()},
            {"capabilities", {}},
            {"trace", "off"}
        }}
    };

    std::string str = initialize_params.dump();

    std::cout << "Content-Length:" + std::to_string(str.size()) + "\r" + str + "\n";
    m_process->write("Content-Length: " + std::to_string(str.size()) + "\r" + str + "\n\n");

    nlohmann::json did_open_params = {
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didOpen"},
        {"params", {
            {"textDocument", {
                {"languageId", "cpp"},
                {"version", 1},
                {"text", "int main() {\n  ;;  return 0;\n}"}
            }}
        }}
    };

    str = did_open_params.dump();
    m_process->write("Content-Length: " + std::to_string(str.size()) + "\r" + str + "\n");
}

LspHandler::~LspHandler(void)
{
    
}

}