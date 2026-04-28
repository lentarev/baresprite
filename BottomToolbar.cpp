#include "BottomToolbar.h"
#include "FramePanel.h"

namespace baresprite
{

BottomToolbar::BottomToolbar(HWND hWnd, HINSTANCE hInstance, AppState &appState) : _hWnd(hWnd), _hInstance(hInstance), _appState(appState)
{

    // Registers the window class for BottomToolbar
    WNDCLASSEXW wcex = {};

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = _BottomToolbarWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    wcex.lpszClassName = L"BareSpriteBottomToolbarClass";

    RegisterClassExW(&wcex);

    // Create a toolbar container window
    _hToolbar = CreateWindowEx(0, L"BareSpriteBottomToolbarClass", L"", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, 0, 0, 0, 0, hWnd, nullptr, hInstance, nullptr);

    if (!_hToolbar)
    {
        return; // Не удалось создать окно
    }

    _framePanel = std::make_unique<FramePanel>(_hToolbar, hInstance, appState);
}

BottomToolbar::~BottomToolbar()
{
    if (_hToolbar)
    {
        DestroyWindow(_hToolbar);
    }
}

void BottomToolbar::OnSize(int clientW, int clientH)
{

    if (_hToolbar)
    {
        // Растягиваем тулбар на всю высоту главного окна
        SetWindowPos(_hToolbar, nullptr, 0, clientH - HEIGHT, clientW, HEIGHT, SWP_NOZORDER | SWP_NOACTIVATE);
    }

    if (_framePanel)
    {
        _framePanel->OnSize(clientW, clientH);
    }
}

bool BottomToolbar::OnCommand(int commandId, int notifyCode)
{
    return false;
}

LRESULT CALLBACK BottomToolbar::_BottomToolbarWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // Forwarding notifications from child controls to the parent
    if (message == WM_COMMAND || message == WM_NOTIFY)
    {
        HWND hParent = GetParent(hWnd);
        if (hParent)
        {
            SendMessage(hParent, message, wParam, lParam);
        }
        return 0; // The message has been processed.
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

} // namespace baresprite
