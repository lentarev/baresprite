#include "export_spritesheet_dialog.h"
#include "ExportSequenceData.h"
#include "SpritesheetService.h"
#include "export_spritesheet_dialog_proc.h"
#include "resource.h"

namespace baresprite
{

bool ExportSpritesheetDialog(HINSTANCE hInstance, HWND hWnd, AppState &appState)
{
    if (appState.frames.empty())
    {
        MessageBox(hWnd, L"No frames to export.", L"Error", MB_OK | MB_ICONWARNING);
        return false;
    }

    ExportSequenceData exportData;
    exportData.appState = &appState;
    exportData.confirmed = false;

    INT_PTR result =
        DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_EXPORT_SPRITESHEET_DIALOG), hWnd, ExportSpritesheetDialogProc, reinterpret_cast<LPARAM>(&exportData));

    if (result == IDOK && exportData.confirmed)
    {
        bool success = SpritesheetService::BuildSpritesheet(exportData);

        if (success)
        {
            std::wstring msg =
                L"Sprite Sheet exported:\n" + exportData.fileName + L".png" + (exportData.generateJson ? L"\n" + exportData.fileName + L".json" : L"");
            MessageBox(hWnd, msg.c_str(), L"Success", MB_OK | MB_ICONINFORMATION);
        }
        else
        {
            MessageBox(hWnd, L"Failed to export Sprite Sheet.", L"Error", MB_OK | MB_ICONERROR);
        }

        return success;
    }

    return false;
}

} // namespace baresprite
