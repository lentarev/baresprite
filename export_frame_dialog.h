#pragma once

#include "AppState.h"
#include <Windows.h>

namespace baresprite
{

/// <summary>
/// Exporting a single frame dialog
/// </summary>
/// <param name="hWnd"></param>
/// <param name="appState"></param>
/// <returns></returns>
bool ExportFrameDialog(HWND hWnd, AppState &appState);

} // namespace baresprite