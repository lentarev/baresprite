#include "export_frame_dialog.h"
#include "ExportService.h"
#include "Frame.h"

namespace baresprite
{

/// <summary>
/// Exporting a single frame dialog
/// </summary>
/// <param name="hWnd"></param>
/// <param name="appState"></param>
/// <returns></returns>
bool ExportFrameDialog(HWND hWnd, AppState &appState)
{
    if (appState.frames.empty())
    {
        MessageBox(hWnd, L"No frames to export.", L"Error", MB_OK | MB_ICONWARNING);
        return 0;
    }

    const Frame &currentFrame = appState.frames[appState.currentFrameIndex];

    // We select the buffer immediately and fill it with the initial name
    std::wstring filePath(MAX_PATH, L'\0');
    swprintf_s(&filePath[0], MAX_PATH, L"frame_%03d.png", appState.currentFrameIndex);

    OPENFILENAMEW ofn = {};

    ofn.lStructSize = sizeof(OPENFILENAMEW);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = L"PNG Images\0*.png\0All Files\0*.*\0";
    ofn.lpstrFile = &filePath[0];
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = L"png";
    ofn.lpstrTitle = L"Export Frame as PNG";

    if (GetSaveFileNameW(&ofn))
    {
        // Trim off any extra zeros left over from initialization.
        filePath.resize(wcslen(&filePath[0]));

        if (ExportService::ExportFrameToPNG(currentFrame, filePath))
        {
            MessageBox(hWnd, L"Frame exported successfully.", L"Success", MB_OK | MB_ICONINFORMATION);
        }
        else
        {
            MessageBox(hWnd, L"Failed to export frame.", L"Error", MB_OK | MB_ICONERROR);
        }
    }

    return 0;
}

} // namespace baresprite