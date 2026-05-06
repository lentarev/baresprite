#pragma once

#include <Windows.h>

namespace baresprite
{

// Message handler for New Project Dialog box.
INT_PTR CALLBACK LoadProjectDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

} // namespace baresprite
