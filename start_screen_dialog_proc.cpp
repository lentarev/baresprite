#include "start_screen_dialog_proc.h"
#include "resource.h"

namespace baresprite
{

// Message handler for Start Screen Dialog box.
INT_PTR CALLBACK StartScreenDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDC_BUTTON_LOAD_PROJECT)
        {

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }

        break;

    case WM_CLOSE:
        EndDialog(hDlg, IDCANCEL);
        return (INT_PTR)TRUE;
    }

    return (INT_PTR)FALSE;
}

} // namespace baresprite
