#include "export_sequence_dialog_proc.h"
#include "ExportSequenceData.h"
#include "resource.h"
#include <algorithm>
#include <unordered_set>
#include <windowsx.h>

#include <shlobj.h> // Browse folder

namespace baresprite
{
/// <summary>
/// Message handler for Export Sequence Dialog box
/// </summary>
/// <param name="hDlg"></param>
/// <param name="message"></param>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
INT_PTR CALLBACK ExportSequenceDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto *pData = reinterpret_cast<ExportSequenceData *>(GetWindowLongPtr(hDlg, GWLP_USERDATA));

    switch (message)
    {

    case WM_INITDIALOG: {
        pData = reinterpret_cast<ExportSequenceData *>(lParam);
        SetWindowLongPtr(hDlg, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pData));

        if (!pData)
        {

            return TRUE;
        }

        // Centered
        {
            RECT rcParent, rcDlg;
            // We get the coordinates of the main window (parent)
            GetWindowRect(GetParent(hDlg), &rcParent);

            // We get the coordinates of the dialogue itself
            GetWindowRect(hDlg, &rcDlg);

            // Calculating the width and height of a dialog
            int dlgW = rcDlg.right - rcDlg.left;
            int dlgH = rcDlg.bottom - rcDlg.top;

            // Calculate coordinates for centering
            int x = rcParent.left + (rcParent.right - rcParent.left) / 2 - dlgW / 2;
            int y = rcParent.top + (rcParent.bottom - rcParent.top) / 2 - dlgH / 2;

            // Apply the position
            SetWindowPos(hDlg, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
        }

        // >>>>>>>>>>>>>>>>>>>>>>>>>>
        // Output Folder
        HWND hEdit = GetDlgItem(hDlg, IDC_EDIT_OUTPUT_FOLDER);

        if (pData->appState->isExistAppConfig && !pData->appState->projectPath.empty())
        {
            SetWindowTextW(hEdit, pData->appState->projectPath.c_str());

            // Init path to save folder
            pData->outputFolder = pData->appState->projectPath;
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

        // >>>>>>>>>>>>>>>>>>>>>>>>>>
        // Dropdown List
        HWND hCombo = GetDlgItem(hDlg, IDC_COMBO_TAG);

        // Всегда первый: All
        ComboBox_AddString(hCombo, L"All");

        // Collecting used tags
        std::unordered_set<std::wstring> usedTags;
        bool hasNone = false;

        for (const auto &frame : pData->appState->frames)
        {
            std::wstring tag = frame.tag.empty() ? L"None" : frame.tag;
            if (tag != L"All")
            {
                if (tag == L"None")
                    hasNone = true;
                usedTags.insert(tag);
            }
        }

        // Second: None (if there are frames without a tag)
        if (hasNone)
        {
            ComboBox_AddString(hCombo, L"None");
            usedTags.erase(L"None"); // We remove them to avoid duplication when sorting.
        }

        // The rest: strictly alphabetically
        std::vector<std::wstring> sortedTags(usedTags.begin(), usedTags.end());
        std::sort(sortedTags.begin(), sortedTags.end());

        for (const auto &tag : sortedTags)
        {
            ComboBox_AddString(hCombo, tag.c_str());
        }

        // By default, All is selected.
        ComboBox_SetCurSel(hCombo, 0);

        return TRUE;
    }

    case WM_COMMAND: {
        switch (LOWORD(wParam))
        {
        
        // Handle button (Browse)
        case IDC_BTN_BROWSE_FOLDER: {

            // Get the current path from the input field
            HWND hEdit = GetDlgItem(hDlg, IDC_EDIT_OUTPUT_FOLDER);
            wchar_t currentPath[MAX_PATH] = {0};
            GetWindowTextW(hEdit, currentPath, MAX_PATH);

            BROWSEINFOW bi = {};
            bi.hwndOwner = hDlg;
            bi.lpszTitle = L"Select Output Folder";
            bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
            bi.lpfn = [](HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM pData) -> int {
                UNREFERENCED_PARAMETER(lParam);

                if (uMsg == BFFM_INITIALIZED && pData && wcslen((wchar_t *)pData) > 0)
                {
                    // Select the folder, but do not block navigation
                    SendMessageW(hwnd, BFFM_SETSELECTIONW, TRUE, pData);
                }
                return 0;
            };
            bi.lParam = reinterpret_cast<LPARAM>(currentPath);

            PIDLIST_ABSOLUTE pidl = SHBrowseForFolderW(&bi);

            if (pidl != nullptr)
            {
                wchar_t path[MAX_PATH];
                if (SHGetPathFromIDListW(pidl, path))
                {
                    pData->outputFolder = path;
                    SetDlgItemTextW(hDlg, IDC_EDIT_OUTPUT_FOLDER, path);
                }
                CoTaskMemFree(pidl);
            }

            return TRUE;
        }

        case IDOK: {
            wchar_t tagBuf[64];
            GetDlgItemTextW(hDlg, IDC_COMBO_TAG, tagBuf, _countof(tagBuf));
            pData->selectedTag = tagBuf;

            if (pData->outputFolder.empty())
            {
                MessageBox(hDlg, L"Please select an output folder.", L"Error", MB_OK | MB_ICONWARNING);
                return TRUE;
            }

            pData->confirmed = true;
            EndDialog(hDlg, IDOK);
            return TRUE;
        }

        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }
    }
    return FALSE;
}

} // namespace baresprite