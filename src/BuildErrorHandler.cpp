#include "BuildErrorHandler.h"
#include "EditorWindow.h"

namespace Crystal
{

void BuildErrorHandler::ApplyErrorMarkersToAllEditorWindows(std::vector<std::shared_ptr<WindowPlugin>> windows, const std::string& compilerOutput, CompilerType compiler)
{
    switch (compiler)
    {
    case CompilerType::MSBuild:
        m_errors = ParseMSBuildErrors(compilerOutput);
        break;
    /*case CompilerType::GCC:
        m_errors = ParseGCCErrors(compilerOutput);
        break;*/
    }

    for (std::shared_ptr<WindowPlugin> window : windows)
	{
		std::shared_ptr<EditorWindow> editorWindow = std::dynamic_pointer_cast<EditorWindow>(window);

		if (!editorWindow)
            continue;

        std::vector<BuildError> filteredErrors = FilterErrorsByPath(editorWindow->GetFilePath());

        if (filteredErrors.empty())
        {
            editorWindow->GetTextEditor()->SetErrorMarkers({});
            continue;
        }

        TextEditor::ErrorMarkers markers;
        for (const auto &error : filteredErrors)
            markers.insert(std::make_pair(error.m_lineNumber - 1, error.m_errorMessage));
        editorWindow->GetTextEditor()->SetErrorMarkers(markers);
	}
    m_hasAppliedErrorMarkers = true;
}

std::vector<BuildErrorHandler::BuildError> BuildErrorHandler::FilterErrorsByPath(const std::vector<BuildErrorHandler::BuildError> &allErrors, const std::filesystem::path &targetPath)
{
    std::vector<BuildError> filteredErrors;

    for (const auto &error : allErrors)
        if (error.m_filePath == targetPath.string())
            filteredErrors.push_back(error);

    return filteredErrors;
}

std::vector<BuildErrorHandler::BuildError> BuildErrorHandler::ParseMSBuildErrors(const std::string& msbuildOutput)
{
    std::vector<BuildError> errors;

    std::regex errorPattern(R"((.+)\((\d+),\d+\):\s+(error) (.+)\s+\[)");

    std::sregex_iterator iter(msbuildOutput.begin(), msbuildOutput.end(), errorPattern);
    std::sregex_iterator end;

    while (iter != end)
    {
        std::smatch match = *iter;
        std::string filePath = match[1].str();
        int lineNumber = std::stoi(match[2].str());
        std::string errorMessage = match[4].str();

        filePath.erase(filePath.begin(), std::find_if(filePath.begin(), filePath.end(), [](int ch)
        {
            return !std::isspace(ch);
        }));

        BuildError error = { filePath, errorMessage, lineNumber };
        errors.push_back(error);

        ++iter;
    }

    return errors;
}

}