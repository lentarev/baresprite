#include "export_spritesheet_dialog_proc.h"
#include "ExportSequenceData.h"
#include "resource.h"
#include <algorithm>
#include <unordered_set>
#include <vector>
#include <windowsx.h>

#include <shlobj.h> // Browse folder

namespace baresprite
{

INT_PTR CALLBACK ExportSpritesheetDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
        HWND hEditFolder = GetDlgItem(hDlg, IDC_ESS_EDIT_OUTPUT_FOLDER);
        if (pData->appState->isExistAppConfig && !pData->appState->projectPath.empty())
        {
            SetWindowTextW(hEditFolder, pData->appState->projectPath.c_str());
            pData->outputFolder = pData->appState->projectPath;
        }
        else
        {
            PWSTR docsPath = nullptr;
            if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &docsPath)))
            {
                std::wstring defaultPath = std::wstring(docsPath) + L"\\MySpriteProject";
                SetWindowTextW(hEditFolder, defaultPath.c_str());
                CoTaskMemFree(docsPath);
            }
            else
            {
                SetWindowTextW(hEditFolder, L"C:\\Projects\\MySpriteProject");
            }
        }

        // >>>>>>>>>>>>>>>>>>>>>>>>>>
        // Dropdown List - Tag Filter
        HWND hCombo = GetDlgItem(hDlg, IDC_ESS_COMBO_TAG_FILTER);

        // Всегда первый: All
        ComboBox_AddString(hCombo, L"All");

        // Собираем используемые теги
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

        // Второй: None (если есть кадры без тега)
        if (hasNone)
        {
            ComboBox_AddString(hCombo, L"None");
            usedTags.erase(L"None"); // Убираем, чтобы не дублировать при сортировке
        }

        // Остальные: строго по алфавиту
        std::vector<std::wstring> sortedTags(usedTags.begin(), usedTags.end());
        std::sort(sortedTags.begin(), sortedTags.end());

        for (const auto &tag : sortedTags)
        {
            ComboBox_AddString(hCombo, tag.c_str());
        }

        // По умолчанию выбран All
        ComboBox_SetCurSel(hCombo, 0);

        // >>>>>>>>>>>>>>>>>>>>>>>>>>
        // Default values
        SetDlgItemInt(hDlg, IDC_ESS_EDIT_COL, 1, FALSE);           // 1 column by default
        SetDlgItemInt(hDlg, IDC_ESS_EDIT_PAD_PIXELS, 0, FALSE);    // 0 padding
        CheckDlgButton(hDlg, IDC_ESS_CHECK_GEN_JSON, BST_CHECKED); // JSON by default

        return TRUE;
    }

    case WM_COMMAND: {
        switch (LOWORD(wParam))
        {
        case IDC_ESS_BUTTON_BROWSE: {

            BROWSEINFOW bi = {};
            bi.hwndOwner = hDlg;
            bi.lpszTitle = L"Select Output Folder";
            bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

            PIDLIST_ABSOLUTE pidl = SHBrowseForFolderW(&bi);

            if (pidl != nullptr)
            {
                wchar_t path[MAX_PATH];
                if (SHGetPathFromIDListW(pidl, path))
                {
                    pData->outputFolder = path;
                    SetDlgItemTextW(hDlg, IDC_ESS_EDIT_OUTPUT_FOLDER, path);
                }
                CoTaskMemFree(pidl);
            }

            return TRUE;
        }

        case IDOK: {

            // Tag
            wchar_t tagBuf[64];
            GetDlgItemTextW(hDlg, IDC_ESS_COMBO_TAG_FILTER, tagBuf, _countof(tagBuf));
            pData->selectedTag = tagBuf;

            // Output Folder
            wchar_t folderBuf[MAX_PATH];
            GetDlgItemTextW(hDlg, IDC_ESS_EDIT_OUTPUT_FOLDER, folderBuf, _countof(folderBuf));
            pData->outputFolder = folderBuf;

            // File Name
            wchar_t nameBuf[256];
            GetDlgItemTextW(hDlg, IDC_ESS_EDIT_FILENAME, nameBuf, _countof(nameBuf));
            pData->fileName = nameBuf;

            if (pData->fileName.empty())
            {
                MessageBox(hDlg, L"Please enter a file name.", L"Error", MB_OK | MB_ICONWARNING);
                return TRUE;
            }

            // Columns
            pData->columns = GetDlgItemInt(hDlg, IDC_ESS_EDIT_COL, nullptr, FALSE);
            if (pData->columns < 1)
            {
                MessageBox(hDlg, L"Columns must be at least 1.", L"Error", MB_OK | MB_ICONWARNING);
                return TRUE;
            }

            // Padding
            pData->padding = GetDlgItemInt(hDlg, IDC_ESS_EDIT_PAD_PIXELS, nullptr, FALSE);

            // Generate JSON
            pData->generateJson = (IsDlgButtonChecked(hDlg, IDC_ESS_CHECK_GEN_JSON) == BST_CHECKED);

            // Validation
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