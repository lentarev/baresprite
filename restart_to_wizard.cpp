#include "restart_to_wizard.h"

namespace baresprite
{

void RestartToWizard(HWND hWnd, const wchar_t *modeArg)
{
    wchar_t exePath[MAX_PATH];

    if (GetModuleFileNameW(nullptr, exePath, MAX_PATH) == 0)
    {
        return;
    }

    // Removing the focus stealing block for the next process
    AllowSetForegroundWindow(-1);

    // Start new instance
    ShellExecuteW(hWnd, L"open", exePath, modeArg, nullptr, SW_SHOWNORMAL);

    // We terminate the current process correctly
    PostQuitMessage(0);
}

} // namespace baresprite