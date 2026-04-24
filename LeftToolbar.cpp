#include "LeftToolbar.h"
#include "Palette.h"
#include "Tools.h"
#include <iostream>

namespace baresprite
{

LeftToolbar::LeftToolbar(HWND hWndParent, HINSTANCE hInstanceParent, AppState &appState)
    : _hWndParent(hWndParent), _hInstanceParent(hInstanceParent), _appState(appState)
{

   

    // Registers the window class for canvas
    WNDCLASSEXW wcex = {};

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = _LeftToolbarWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstanceParent;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    wcex.lpszClassName = L"BareSpriteLeftToolbarClass";

    RegisterClassExW(&wcex);

    // Create a toolbar container window
    _hToolbar = CreateWindowEx(0, L"BareSpriteLeftToolbarClass", L"", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, 0, 0, WIDTH, 0, hWndParent, nullptr,
                               hInstanceParent, nullptr);

    // Create palette
    _palette = std::make_unique<Palette>(_hToolbar, hInstanceParent, appState);

    // Create button tools
    _tools = std::make_unique<Tools>(_hToolbar, hInstanceParent, appState);

}

LeftToolbar::~LeftToolbar()
{
    if (_hToolbar)
    {
        DestroyWindow(_hToolbar);
    }
}

void LeftToolbar::OnSize(int clientW, int clientH)
{
    if (_hToolbar)
    {
        // Stretch the toolbar to the full height of the main window
        SetWindowPos(_hToolbar, nullptr, 0, 0, WIDTH, clientH, SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

bool LeftToolbar::OnCommand(int commandId, int notifyCode)
{

    // Palette: ID from 3001 to 3025
    if (commandId >= 3001 && commandId < 3001 + _palette->ColorsCount())
    {
        int index = commandId - 3001;

        if (_palette)
        {
            // Select color
            if (notifyCode == BN_CLICKED)
            {
                _palette->SelectColor(index); // Visually highlight

                _appState.palette.color = _palette->GetSelectedColor();
                _appState.palette.index = index;
            }

            // Double click (edit color)
            if (notifyCode == BN_DBLCLK)
            {

                // Open the color selection dialog
                if (_palette->EditColor(index, _hWndParent))
                {
                    // If the color has changed, update the selection
                    _palette->SelectColor(index);

                    // Updating project data
                    _appState.palette.color = _palette->GetSelectedColor();
                    _appState.palette.index = index;

                    // Assign new color
                    _appState.palette.colors[index] = _palette->GetSelectedColor();
                }
            }

            // We note that the project data has been changed.
            _appState.isDirty = true;

            return true;
        }
    }

    return false;
}

LRESULT CALLBACK LeftToolbar::_LeftToolbarWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
