#include "CanvasScrollView.h"
#include "Canvas.h"
#include <iostream>

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
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
    wcex.lpszClassName = L"CanvasScrollViewClass";

    RegisterClassExW(&wcex);

    // Create a scroll view container for canvas window
    _hCanvasScrollView = CreateWindowExW(0, L"CanvasScrollViewClass", L"", (DWORD)(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_HSCROLL | WS_VSCROLL), 0, 0,
                                         _containerW, _containerH, hWndMain, nullptr, hInstance, this);

    // Create canvas
    _canvas = std::make_unique<Canvas>(_hCanvasScrollView, hInstance, appState);

    if (_canvas)
    {
        // We pass a raw pointer to the appState structure to access the canvas.
        appState.canvas = _canvas.get();
    }
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

        if (_canvas)
        {
            _canvas->OnSize(freeAreaW, freeAreaH);

            // Получаем размер canvas
            HWND hCanvas = _canvas->GetHWndCanvas();
            RECT rc;
            GetClientRect(hCanvas, &rc);
            int canvasW = rc.right;
            int canvasH = rc.bottom;

            // Вычисляем начальную позицию с центрированием
            _canvasPosX = 0;
            _canvasPosY = 0;

            // Если canvas меньше контейнера — центрируем
            if (canvasW < _containerW)
            {
                _canvasPosX = (_containerW - canvasW) / 2;
            }
            if (canvasH < _containerH)
            {
                _canvasPosY = (_containerH - canvasH) / 2;
            }

            // Устанавливаем позицию
            SetWindowPos(hCanvas, nullptr, _canvasPosX, _canvasPosY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

            // Сбрасываем скролл
            _scrollX = 0;
            _scrollY = 0;

            UpdateScrollInfo();
        }
    }
}

bool CanvasScrollView::OnCommand(int commandId, int notifyCode)
{
    return false;
}

void CanvasScrollView::UpdateScrollInfo()
{
    if (!_canvas || !_hCanvasScrollView)
        return;

    HWND hCanvas = _canvas->GetHWndCanvas();

    // Получаем размер клиентской области canvas
    RECT rc;
    GetClientRect(hCanvas, &rc);
    int canvasW = rc.right;
    int canvasH = rc.bottom;

    SCROLLINFO si = {};
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;

    si.nMin = 0;
    si.nMax = canvasW - 1;
    si.nPage = _containerW;
    si.nPos = _scrollX;
    SetScrollInfo(_hCanvasScrollView, SB_HORZ, &si, TRUE);

    si.nMax = canvasH - 1;
    si.nPage = _containerH;
    si.nPos = _scrollY;
    SetScrollInfo(_hCanvasScrollView, SB_VERT, &si, TRUE);
}

Canvas *CanvasScrollView::GetCanvas()
{
    return _canvas.get();
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
    case WM_HSCROLL: {
        SCROLLINFO si = {};
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_ALL;
        GetScrollInfo(hWnd, SB_HORZ, &si);

        int newPos = si.nPos;
        switch (LOWORD(wParam))
        {
        case SB_LINEUP:
            newPos -= 16;
            break;
        case SB_LINEDOWN:
            newPos += 16;
            break;
        case SB_PAGEUP:
            newPos -= si.nPage;
            break;
        case SB_PAGEDOWN:
            newPos += si.nPage;
            break;
        case SB_THUMBTRACK:
        case SB_THUMBPOSITION:
            newPos = HIWORD(wParam);
            break;
        }

        // Ограничиваем
        int maxPos = max(0, si.nMax - (int)si.nPage + 1);
        if (newPos < 0)
            newPos = 0;
        if (newPos > maxPos)
            newPos = maxPos;

        if (newPos != si.nPos)
        {
            if (pScrollView->_canvas)
            {
                pScrollView->_scrollX = newPos;

                // Вычисляем начальную позицию (центрирование)
                HWND hCanvas = pScrollView->_canvas->GetHWndCanvas();
                RECT rc;
                GetClientRect(hCanvas, &rc);
                int canvasW = rc.right;

                int initialX = 0;
                if (canvasW < pScrollView->_containerW)
                {
                    initialX = (pScrollView->_containerW - canvasW) / 2;
                }

                // Формула: начальная_позиция - скролл
                pScrollView->_canvasPosX = initialX - newPos;

                SetWindowPos(hCanvas, nullptr, pScrollView->_canvasPosX, pScrollView->_canvasPosY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
            }

            si.nPos = newPos;
            SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);
        }
        return 0;
    }

    case WM_VSCROLL: {
        SCROLLINFO si = {};
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_ALL;
        GetScrollInfo(hWnd, SB_VERT, &si);

        int newPos = si.nPos;
        switch (LOWORD(wParam))
        {
        case SB_LINEUP:
            newPos -= 16;
            break;
        case SB_LINEDOWN:
            newPos += 16;
            break;
        case SB_PAGEUP:
            newPos -= si.nPage;
            break;
        case SB_PAGEDOWN:
            newPos += si.nPage;
            break;
        case SB_THUMBTRACK:
        case SB_THUMBPOSITION:
            newPos = HIWORD(wParam);
            break;
        }

        // Ограничиваем
        int maxPos = max(0, si.nMax - (int)si.nPage + 1);
        if (newPos < 0)
            newPos = 0;
        if (newPos > maxPos)
            newPos = maxPos;

        if (newPos != si.nPos)
        {
            if (pScrollView->_canvas)
            {
                pScrollView->_scrollY = newPos;

                // Вычисляем начальную позицию (центрирование)
                HWND hCanvas = pScrollView->_canvas->GetHWndCanvas();
                RECT rc;
                GetClientRect(hCanvas, &rc);
                int canvasH = rc.bottom;

                int initialY = 0;
                if (canvasH < pScrollView->_containerH)
                {
                    initialY = (pScrollView->_containerH - canvasH) / 2;
                }

                // Формула: начальная_позиция - скролл
                pScrollView->_canvasPosY = initialY - newPos;

                SetWindowPos(hCanvas, nullptr, pScrollView->_canvasPosX, pScrollView->_canvasPosY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
            }

            si.nPos = newPos;
            SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
        }
        return 0;
    }

    case WM_ERASEBKGND:
        HDC hdc = (HDC)wParam;
        RECT rc;
        GetClientRect(hWnd, &rc);

        HBRUSH hBrush = CreateSolidBrush(RGB(200, 200, 200));
        FillRect(hdc, &rc, hBrush);
        DeleteObject(hBrush);
        return TRUE;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

} // namespace baresprite