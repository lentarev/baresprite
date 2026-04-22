#include "LeftToolbar.h"
#include "Palette.h"
#include <iostream>

namespace baresprite
{

LeftToolbar::LeftToolbar(HWND hWndParent, HINSTANCE hInstanceParent, Project &projectData)
    : _hWndParent(hWndParent), _hInstanceParent(hInstanceParent), _projectData(projectData)
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
    _palette = std::make_unique<Palette>(_hToolbar, hInstanceParent);
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

bool LeftToolbar::OnCommand(int commandId)
{

    // Palette: ID from 3001 to 3025
    if (commandId >= 3001 && commandId < 3001 + _palette->ColorsCount())
    {
        int index = commandId - 3001;

        if (_palette)
        {
            _palette->SelectColor(index); // Visually highlight
            std::cout << "Palette ID: " << commandId << std::endl;

            COLORREF color = _palette->GetSelectedColor();
            _projectData.paletteColor = color;
        }
        return true;
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
