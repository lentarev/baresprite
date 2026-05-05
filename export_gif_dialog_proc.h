#pragma once

#include <Windows.h>

namespace baresprite
{

// Message handler for Export GIF Dialog box.
INT_PTR CALLBACK ExportGifDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

} // namespace baresprite