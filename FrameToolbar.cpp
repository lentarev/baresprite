#include "FrameToolbar.h"

namespace baresprite
{

FrameToolbar::FrameToolbar(HWND hWndParent, HINSTANCE hInstanceParent) : _hWndParent(hWndParent), _hInstanceParent(hInstanceParent)
{
    // Create a toolbar container window
    _hToolbar =
        CreateWindowEx(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, 0, 0, 0, 0, hWndParent, nullptr, hInstanceParent, nullptr);
}

FrameToolbar::~FrameToolbar()
{
    if (_hToolbar)
    {
        DestroyWindow(_hToolbar);
    }
}

void FrameToolbar::OnSize(int clientW, int clientH)
{


    if (_hToolbar)
    {
        // Растягиваем тулбар на всю высоту главного окна
        SetWindowPos(_hToolbar, nullptr, 0, clientH - HEIGHT, clientW, HEIGHT, SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

bool FrameToolbar::OnCommand(int commandId)
{
    return false;
}

} // namespace baresprite
