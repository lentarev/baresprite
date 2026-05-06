#include "load_project_dialog_proc.h"

#include "AppState.h"
#include "resource.h"

#include <Windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <windowsx.h>

namespace baresprite
{
INT_PTR CALLBACK LoadProjectDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG: {
        // Bring the window to the foreground
        SetForegroundWindow(hDlg);

        SetWindowLongPtr(hDlg, DWLP_USER, lParam);
        AppState *appState = reinterpret_cast<AppState *>(lParam);

        // Location folder
        HWND hEdit = GetDlgItem(hDlg, IDC_EDIT_BROWSE_LOAD_PROJECT);

        if (appState->isExistAppConfig)
        {
            SetWindowTextW(hEdit, appState->projectPath.c_str());
        }
        else
        {
            // Fallback: If the path is empty, use Documents
            PWSTR docsPath = nullptr;
            if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &docsPath)))
            {
                std::wstring defaultPath = std::wstring(docsPath) + L"\\MySpriteProject";

                SetWindowTextW(hEdit, defaultPath.c_str());

                CoTaskMemFree(docsPath); // Freeing up memory!
            }
            else
            {
                SetWindowTextW(hEdit, L"C:\\Projects\\MySpriteProject");
            }
        }

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:

        // Handle button (Browse)
        if (LOWORD(wParam) == IDC_BUTTON_BROWSE_LOAD_PROJECT)
        {
            HWND hBtn = GetDlgItem(hDlg, IDC_BUTTON_BROWSE_LOAD_PROJECT);

            // Lock the button (Browse) to prevent repeated pressings
            EnableWindow(hBtn, FALSE);

            // Get the current path from the input field
            HWND hEdit = GetDlgItem(hDlg, IDC_EDIT_BROWSE_LOAD_PROJECT);
            wchar_t currentPath[MAX_PATH] = {0};
            GetWindowTextW(hEdit, currentPath, MAX_PATH);

            // Structure for dialogue
            BROWSEINFOW bi = {};
            bi.hwndOwner = hDlg; // we indicate the owner of the dialogue
            bi.lpszTitle = L"Choose folder for project:";
            bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_USENEWUI;

            // Callback for folder preselection after initialization
            bi.lpfn = [](HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM pData) -> int {
                UNREFERENCED_PARAMETER(lParam);

                if (uMsg == BFFM_INITIALIZED && pData && wcslen((wchar_t *)pData) > 0)
                {
                    // Select the folder, but do not block navigation
                    SendMessageW(hwnd, BFFM_SETSELECTIONW, TRUE, pData);
                }
                return 0;
            };

            bi.lParam = reinterpret_cast<LPARAM>(currentPath); // Passing the path to the callback

            // Showing the dialogue
            LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);

            // If the user has selected a folder, we update the input field.
            if (pidl)
            {
                wchar_t path[MAX_PATH];
                if (SHGetPathFromIDListW(pidl, path))
                {
                    SetWindowTextW(hEdit, path);
                }
                CoTaskMemFree(pidl); // Freeing up memory
            }

            // Unlock the button (Browse) after closing the dialog
            EnableWindow(hBtn, TRUE);

            return TRUE;
        }

        // Handle button (Ok)
        if (LOWORD(wParam) == IDOK)
        {
            HWND hEdit = GetDlgItem(hDlg, IDC_EDIT_BROWSE_LOAD_PROJECT);

            // Read the pointer
            AppState *appState = reinterpret_cast<AppState *>(GetWindowLongPtr(hDlg, DWLP_USER));

            if (appState)
            {
                // Reading the path
                wchar_t pathBuf[MAX_PATH];
                GetWindowTextW(hEdit, pathBuf, MAX_PATH);

                appState->projectPath = pathBuf;
            }

            EndDialog(hDlg, LOWORD(wParam));

            return (INT_PTR)TRUE;
        }

        // Handle button (Cancel)
        if (LOWORD(wParam) == IDCANCEL)
        {

            EndDialog(hDlg, LOWORD(wParam));

            return (INT_PTR)TRUE;
        }

        break;
    }

    return (INT_PTR)FALSE;
}

} // namespace baresprite