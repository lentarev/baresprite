#include "CanvasScrollView.h"

namespace baresprite
{

CanvasScrollView::CanvasScrollView(HWND hWndMain, HINSTANCE hInstance, AppState &appState) : _hWndMain(hWndMain), _hInstance(hInstance), _appState(appState)
{
    // Registers the window class for CanvasScrollView
    WNDCLASSEXW wcex = {};

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = _CanvasScrollViewWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_CROSS);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
    wcex.lpszClassName = L"CanvasScrollViewClass";

    RegisterClassExW(&wcex);

    // Create a scroll view container for canvas window
    _hCanvasScrollView = CreateWindowExW(0, L"CanvasScrollViewClass", L"", (DWORD)(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS), 0, 0, _containerW, _containerH,
                                         hWndMain, nullptr, hInstance, this);
}

CanvasScrollView::~CanvasScrollView()
{
    if (_hCanvasScrollView)
    {
        DestroyWindow(_hCanvasScrollView);
    }
}

void CanvasScrollView::OnSize(int clientW, int clientH)
{
    if (_hCanvasScrollView)
    {
        const int PADDING = 5;

        const int freeAreaW = (clientW - LEFT_TOOLBAR_WIDTH - RIGHT_TOOLBAR_WIDTH);
        const int freeAreaH = (clientH - FRAME_TOOLBAR_HEIGHT);

        _containerW = freeAreaW - PADDING;
        _containerH = freeAreaH - PADDING;

        _offsetX = LEFT_TOOLBAR_WIDTH + (freeAreaW - _containerW) / 2;
        _offsetY = (freeAreaH - _containerH) / 2;

        SetWindowPos(_hCanvasScrollView, nullptr, _offsetX, _offsetY, _containerW, _containerH, SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

bool CanvasScrollView::OnCommand(int commandId, int notifyCode)
{
    return false;
}

LRESULT CALLBACK CanvasScrollView::_CanvasScrollViewWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    CanvasScrollView *pScrollView = reinterpret_cast<CanvasScrollView *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    if (message == WM_NCCREATE)
    {

        CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT *>(lParam);
        pScrollView = reinterpret_cast<CanvasScrollView *>(pCreate->lpCreateParams);

        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pScrollView));
    }

    // If the object is still not there -> we give it to the system
    if (!pScrollView)
    {
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    switch (message)
    {
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

} // namespace baresprite