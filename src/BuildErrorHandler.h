#pragma once

#include "WindowManager.h"

#include <string>
#include <regex>
#include <vector>

namespace Crystal
{
class BuildErrorHandler
{
public:
    enum CompilerType
    {
        MSBuild,
        //GCC
    };

    struct BuildError
    {
        std::string m_filePath;
        std::string m_errorMessage;
        int m_lineNumber;
    };

    void ApplyErrorMarkersToAllEditorWindows(WindowManager &wm, const std::string& compilerOutput, CompilerType compiler);

    std::vector<BuildError> FilterErrorsByPath(const std::vector<BuildError> &allErrors, const std::filesystem::path& targetPath);
    std::vector<BuildError> FilterErrorsByPath(const std::filesystem::path &targetPath) { return FilterErrorsByPath(m_errors, targetPath); }

    bool HasAppliedErrorMarkers(void) { return m_hasAppliedErrorMarkers; }

private:
    std::vector<BuildError> ParseMSBuildErrors(const std::string& msbuildOutput);
    std::vector<BuildError> m_errors;
    
    bool m_hasAppliedErrorMarkers = false;
};
}
