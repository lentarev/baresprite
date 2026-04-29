#include "BottomToolbar.h"
#include "FramePanel.h"
#include "TagPanel.h"
#include <iostream>

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
    _hToolbar = CreateWindowEx(0, L"BareSpriteBottomToolbarClass", L"", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, 0, 0, 0, hWnd, nullptr,
                               hInstance, nullptr);

    if (!_hToolbar)
    {
        return; // Не удалось создать окно
    }

    _framePanel = std::make_unique<FramePanel>(_hToolbar, hInstance, appState);
    _tagPanel = std::make_unique<TagPanel>(_hToolbar, hInstance, appState);
}

BottomToolbar::~BottomToolbar()
{
    if (_hToolbar)
    {
        DestroyWindow(_hToolbar);
    }
}

FramePanel *BottomToolbar::GetFramePanel() const
{
    return _framePanel.get();
}

void BottomToolbar::OnSize(int clientW, int clientH)
{

    if (_hToolbar)
    {
        SetWindowPos(_hToolbar, nullptr, 0, clientH - HEIGHT, clientW, HEIGHT, SWP_NOZORDER | SWP_NOACTIVATE);
    }

    if (_framePanel)
    {
        RECT rcFrame = {0, 0, clientW, HEIGHT};
        _framePanel->SetBounds(rcFrame);
    }

    if (_tagPanel && _framePanel)
    {

        int tagPanelStartX = _framePanel->GetRightEdge() + 20; // 20px отступ между панелями
        RECT rcTag = {tagPanelStartX, 0, clientW, HEIGHT};
        _tagPanel->SetBounds(rcTag);
    }
}

bool BottomToolbar::OnCommand(int commandId, int notifyCode)
{
    if (commandId >= 3041 && commandId < 3041 + 5)
    {
        const int index = commandId - 3041;

        if (notifyCode == BN_CLICKED)
        {
            // Prev <
            if (index == 0)
            {
                return _framePanel->OnButtonPrev();
            }
            // Next >
            else if (index == 1)
            {
                return _framePanel->OnButtonNext();
            }
            // New
            else if (index == 2)
            {
                return _framePanel->OnButtonNew();
            }
            // Clone
            else if (index == 3)
            {
                return _framePanel->OnButtonClone();
            }
            // Delete
            else if (index == 4)
            {
                return _framePanel->OnButtonDelete();
            }

            std::cout << "index: " << index << std::endl;
        }
    }

    return false;
}

HWND BottomToolbar::GetHWndBottomToolbar() const
{
    return _hToolbar;
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
