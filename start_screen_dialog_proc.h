#pragma once

#include <Windows.h>

namespace baresprite
{

// Message handler for Start Screen Dialog box.
INT_PTR CALLBACK StartScreenDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

} // namespace baresprite
