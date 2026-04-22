#include "ask_save_dialog.h"

namespace baresprite
{

/// <summary>
/// Save changes before closing dialog
/// </summary>
/// <param name="hWnd"></param>
/// <returns></returns>
bool AskSaveDialog(HWND hWnd, Project &projectData, ProjectSettings &projectSettings)
{
    // If there are no changes, we immediately allow closure.
    if (!projectData.isDirty)
    {
        return true;
    }

    int result = MessageBox(hWnd, L"Save changes before closing?", L"Confirm", MB_YESNOCANCEL | MB_ICONWARNING);

    if (result == IDYES)
    {

        projectSettings.Save();

        // IDYES. Close
        return true;
    }
    else if (result == IDCANCEL)
    {
        return false; // Cancel
    }

    // IDNO. Close
    return true;
}

} // namespace baresprite
