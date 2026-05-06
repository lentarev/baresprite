#pragma once

#include <Windows.h>

namespace baresprite
{

// Message handler for Export Sequence Dialog box.
INT_PTR CALLBACK ExportSequenceDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

}