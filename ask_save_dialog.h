#pragma once

#include "AppState.h"
#include "ProjectSettings.h"
#include <Windows.h>

namespace baresprite
{
/// <summary>
/// Save changes before closing dialog
/// </summary>
/// <returns></returns>
bool AskSaveDialog(HWND hWnd, AppState &appState, ProjectSettings &projectSettings);

} // namespace baresprite