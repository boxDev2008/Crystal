#pragma once

#include "WindowManager.h"
#include <filesystem>

namespace Crystal
{
class DragDropHandler
{
public:
    DragDropHandler(void) = default;
    DragDropHandler(Application *application) : m_application(application) {}
    void BeginDragDropFileMoveTarget(const std::filesystem::path &path);
    void BeginDragDropFileOpenTarget(void);

private:
    Application *m_application;
};
}
