#include "reorder_frames_dlg.h"
#include "reorder_frames_dlg_proc.h"
#include "resource.h"
#include "Canvas.h"

namespace baresprite
{

bool ReorderFramesDlg(HINSTANCE hInstance, HWND hWnd, AppState &appState)
{
    if (appState.frames.empty())
    {
        MessageBox(hWnd, L"No frames to sort.", L"Error", MB_OK | MB_ICONWARNING);
        return false;
    }

    ReorderDlgData dlgData;
    dlgData.appState = &appState;
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG_REORDER_FRAMES), hWnd, ReorderFramesDlgProc, reinterpret_cast<LPARAM>(&dlgData));

    if (appState.canvas)
        InvalidateRect(appState.canvas->GetHWndCanvas(), nullptr, FALSE);

    return true;
}

} // namespace baresprite