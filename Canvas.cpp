#include "Canvas.h"
#include <iostream>

namespace baresprite
{
Canvas::Canvas(HWND hWndParent, HINSTANCE hInstanceParent) : _hWndParent(hWndParent), _hInstanceParent(hInstanceParent)
{

    // Registers the window class for canvas
    WNDCLASSEXW wcex = {};

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = _CanvasWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstanceParent;
    wcex.hCursor = LoadCursor(nullptr, IDC_CROSS);
    wcex.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    wcex.lpszClassName = L"BareSpriteCanvasClass";

    RegisterClassExW(&wcex);

    // Create a canvas window
    _hCanvas = CreateWindowExW((DWORD)WS_EX_CLIENTEDGE, L"BareSpriteCanvasClass", L"", (DWORD)(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS), 0, 0, _canvasWidth,
                               _canvasHeight, hWndParent, nullptr, hInstanceParent, this);

    if (!_hCanvas)
    {
        return; // Не удалось создать окно
    }
}

Canvas::~Canvas()
{
    if (_hCanvas)
    {
        DestroyWindow(_hCanvas);
    }
}

void Canvas::OnSize(int clientW, int clientH)
{
    if (_hCanvas)
    {

        const int canvasAreaWidth = (clientW - LEFT_TOOLBAR_WIDTH - RIGHT_TOOLBAR_WIDTH);
        const int canvasAreaHeight = (clientH - FRAME_TOOLBAR_HEIGHT);

        _offsetX = LEFT_TOOLBAR_WIDTH + (canvasAreaWidth - _canvasWidth) / 2;
        _offsetY = (canvasAreaHeight - _canvasHeight) / 2;

        SetWindowPos(_hCanvas, nullptr, _offsetX, _offsetY, _canvasWidth, _canvasHeight, SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

bool Canvas::OnCommand(int commandId)
{
    return false;
}

LRESULT CALLBACK Canvas::_CanvasWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    Canvas *pCanvas = reinterpret_cast<Canvas *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    if (message == WM_NCCREATE)
    {

        CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT *>(lParam);
        pCanvas = reinterpret_cast<Canvas *>(pCreate->lpCreateParams);

        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCanvas));
    }

    // If the object is still not there -> we give it to the system
    if (!pCanvas)
    {
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    switch (message)
    {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // TODO: The drawing will be done here:
        // 1. Chess background
        // 2. Sprite from gProjectData
        // 3. Grid (if zoom >= 4x)

        // For now, just fill it with white for the dough.
        RECT rc;
        GetClientRect(hWnd, &rc);
        FillRect(hdc, &rc, (HBRUSH)(COLOR_WINDOW + 1));

        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_ERASEBKGND:
        // Preventing flickering
        return TRUE;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

} // namespace baresprite
