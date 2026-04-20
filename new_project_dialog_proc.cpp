#include "new_project_dialog_proc.h"

#include "Project.h"
#include "resource.h"

#include <Windows.h>
#include <windowsx.h>
#include <shlobj.h>  
#include <iostream>

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

        if (projectData->isExistAppConfig)
        {
            std::cout << "config is found: SUCCESS" << std::endl;
        }
        else
        {
            std::cout << "config not found" << std::endl;
        }

        // Filling the ComboBox
        HWND hCombo = GetDlgItem(hDlg, IDC_COMBO_NEW_PROJECT_RESOLUTION);

        ComboBox_AddString(hCombo, L"64 x 64 pixels (Recommended)");
        ComboBox_AddString(hCombo, L"128 x 128 pixels");
        ComboBox_SetCurSel(hCombo, 0);

        // Location folder
        HWND hEdit = GetDlgItem(hDlg, IDC_EDIT_NEW_PROJECT_BROWSE);

        if (projectData->isExistAppConfig)
        {
            SetWindowTextW(hEdit, projectData->configPath.c_str());
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
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }

        break;
    }
    return (INT_PTR)FALSE;
}

} // namespace baresprite
