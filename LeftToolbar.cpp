#include "LeftToolbar.h"


namespace baresprite
{

LeftToolbar::LeftToolbar(const HWND &hWndParent, const HINSTANCE &hInstanceParent) : _hWndParent(hWndParent), _hInstanceParent(hInstanceParent)
{
    // Create a toolbar container window
    _hToolbar = CreateWindowEx(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, 0, 0, WIDTH, 0, hWndParent, nullptr, hInstanceParent, nullptr);
}

LeftToolbar::~LeftToolbar()
{
    if (_hToolbar)
    {
        DestroyWindow(_hToolbar);
    }
}

void LeftToolbar::OnSize(int width, int height)
{
    if (_hToolbar)
    {
        // Растягиваем тулбар на всю высоту главного окна
        SetWindowPos(_hToolbar, nullptr, 0, 0, WIDTH, height, SWP_NOZORDER | SWP_NOACTIVATE);
    }
}


bool LeftToolbar::OnCommand(int commandId)
{
    return false;
}

} // namespace baresprite
