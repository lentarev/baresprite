#include "export_sequence_dialog.h"

#include "ExportSequenceData.h"
#include "export_sequence_dialog_proc.h"
#include "resource.h"
#include "ExportService.h"

namespace baresprite
{

bool ExportSequenceDialog(HINSTANCE hInstance, HWND hWnd, AppState &appState)
{
    if (appState.frames.empty())
    {
        MessageBox(hWnd, L"No frames to export.", L"Error", MB_OK | MB_ICONWARNING);
        return 0;
    }

    ExportSequenceData exportData;
    exportData.appState = &appState;
    exportData.confirmed = false;

    INT_PTR result =
        DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_EXPORT_SEQUENCE_DIALOG), hWnd, ExportSequenceDialogProc, reinterpret_cast<LPARAM>(&exportData));

    if (result == IDOK && exportData.confirmed)
    {
        std::wstring message;

        bool ok = ExportService::ExportSequence(appState.frames, exportData.selectedTag, exportData.outputFolder, message);

        MessageBox(hWnd, message.c_str(), ok ? L"Export Complete" : L"Export Warning", MB_OK | (ok ? MB_ICONINFORMATION : MB_ICONWARNING));
    }

    return 0;
}

} // namespace baresprite