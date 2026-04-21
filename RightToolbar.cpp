#include "RightToolbar.h"

namespace baresprite
{
RightToolbar::RightToolbar(HWND hWndParent, HINSTANCE hInstanceParent) : _hWndParent(hWndParent), _hInstanceParent(hInstanceParent)
{
    // Create a toolbar container window
    _hToolbar = CreateWindowEx(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, 0, 0, WIDTH, 0, hWndParent, nullptr, hInstanceParent, nullptr);
}

RightToolbar::~RightToolbar()
{
    if (_hToolbar)
    {
        DestroyWindow(_hToolbar);
    }
}

void RightToolbar::OnSize(int clientW, int clientH)
{
    if (_hToolbar)
    {
        // Растягиваем тулбар на всю высоту главного окна
        SetWindowPos(_hToolbar, nullptr, clientW - WIDTH, 0, WIDTH, clientH, SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

bool RightToolbar::OnCommand(int commandId)
{
    return false;
}
} // namespace baresprite