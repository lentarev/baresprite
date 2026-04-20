#include "new_project_dialog_proc.h"

#include "Project.h"
#include "resource.h"

#include <Windows.h>
#include <iostream>
#include <shlobj.h>
#include <shlwapi.h>
#include <windowsx.h>

namespace baresprite
{

// Message handler for New Project Dialog box.
INT_PTR CALLBACK NewProjectDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

    switch (message)
    {
    case WM_INITDIALOG: {
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);
        Project *projectData = reinterpret_cast<Project *>(lParam);

        // Filling the ComboBox
        HWND hCombo = GetDlgItem(hDlg, IDC_COMBO_NEW_PROJECT_RESOLUTION);

        ComboBox_AddString(hCombo, L"64 x 64 pixels (Recommended)");
        ComboBox_AddString(hCombo, L"128 x 128 pixels");
        ComboBox_SetCurSel(hCombo, 0);

        // Location folder
        HWND hEdit = GetDlgItem(hDlg, IDC_EDIT_NEW_PROJECT_BROWSE);

        if (projectData->isExistAppConfig)
        {
            SetWindowTextW(hEdit, projectData->projectPath.c_str());
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
        if (LOWORD(wParam) == IDC_BUTTON_NEW_PROJECT_BROWSE)
        {
            // Get the current path from the input field
            HWND hEdit = GetDlgItem(hDlg, IDC_EDIT_NEW_PROJECT_BROWSE);
            wchar_t currentPath[MAX_PATH] = {0};
            GetWindowTextW(hEdit, currentPath, MAX_PATH);

            // Structure for dialogue
            BROWSEINFOW bi = {};
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

            return TRUE;
        }

        // Handle button (Ok)
        if (LOWORD(wParam) == IDOK)
        {
            HWND hCombo = GetDlgItem(hDlg, IDC_COMBO_NEW_PROJECT_RESOLUTION);
            HWND hEdit = GetDlgItem(hDlg, IDC_EDIT_NEW_PROJECT_BROWSE);

            // Read the pointer
            Project *projectData = reinterpret_cast<Project *>(GetWindowLongPtr(hDlg, DWLP_USER));

            if (projectData)
            {

                // Resolution
                int sel = ComboBox_GetCurSel(hCombo);

                if (sel == 0)
                {
                    projectData->imageSize = 64;
                    projectData->checkerSize = 8;
                }
                else
                {
                    projectData->imageSize = 128;
                    projectData->checkerSize = 4;
                }

                // Reading the path
                wchar_t pathBuf[MAX_PATH];
                GetWindowTextW(hEdit, pathBuf, MAX_PATH);

                projectData->projectPath = pathBuf;
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
