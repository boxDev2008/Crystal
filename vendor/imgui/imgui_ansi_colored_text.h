#pragma once

#include "imgui.h"

namespace ImGui
{
    void TextAnsiSetPalette16(unsigned int *colors);
    void TextAnsiUnformatted(const char* text, const char* text_end);
    void TextAnsiV(const char* fmt, va_list args);
    void TextAnsiColoredV(const ImVec4& col, const char* fmt, va_list args);
    void TextAnsiColored(const ImVec4& col, const char* fmt, ...);
}