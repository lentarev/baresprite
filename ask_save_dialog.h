#pragma once

#include <Windows.h>
#include "Project.h"
#include "ProjectSettings.h"

namespace baresprite
{
/// <summary>
/// Save changes before closing dialog
/// </summary>
/// <returns></returns>
bool AskSaveDialog(HWND hWnd, Project &projectData, ProjectSettings &projectSettings);

}