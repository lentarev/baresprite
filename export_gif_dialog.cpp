#include "export_gif_dialog.h"
#include "ExportSequenceData.h"
#include "resource.h"
#include "export_gif_dialog_proc.h"
#include "GifExportService.h"

namespace baresprite
{

bool ExportGifDialog(HINSTANCE hInstance, HWND hWnd, AppState &appState)
{
    if (appState.frames.empty())
    {
        MessageBox(hWnd, L"No frames to export.", L"Error", MB_OK | MB_ICONWARNING);
        return false;
    }

    ExportSequenceData exportData;
    exportData.appState = &appState;
    exportData.confirmed = false;

    INT_PTR result = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG_EXPORT_GIF), hWnd, ExportGifDialogProc, reinterpret_cast<LPARAM>(&exportData));


    if (result == IDOK && exportData.confirmed)
    {
        bool success = GifExportService::ExportGif(exportData); 

        if (success)
        {
            std::wstring msg = L"GIF exported:\n" + exportData.fileName + L".gif";
            MessageBox(hWnd, msg.c_str(), L"Success", MB_OK | MB_ICONINFORMATION);
        }
        else
        {
            MessageBox(hWnd, L"Failed to export GIF.", L"Error", MB_OK | MB_ICONERROR);
        }

        return success;
    }

    
    return false;
}

} // namespace baresprite
