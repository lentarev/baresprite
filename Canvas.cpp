#include "Canvas.h"
#include "ChessBackground.h"
#include "Frame.h"
#include <iostream>
#include <windowsx.h>

namespace baresprite
{
Canvas::Canvas(HWND hWndParent, HINSTANCE hInstanceParent, AppState &appState) : _hWndParent(hWndParent), _hInstanceParent(hInstanceParent), _appState(appState)
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
    _hCanvas = CreateWindowExW(0, L"BareSpriteCanvasClass", L"", (DWORD)(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS), 0, 0, _canvasWidth, _canvasHeight,
                               hWndParent, nullptr, hInstanceParent, this);

    if (!_hCanvas)
    {
        return; // Не удалось создать окно
    }

    _checkerSize = _appState.checkerSize;

    _chessBackground = std::make_unique<ChessBackground>(appState);
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
        _canvasAreaW = clientW;
        _canvasAreaH = clientH;

        _offsetX = (_canvasAreaW - _canvasWidth) / 2;
        _offsetY = (_canvasAreaH - _canvasHeight) / 2;

        SetWindowPos(_hCanvas, nullptr, _offsetX, _offsetY, _canvasWidth, _canvasHeight, SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

bool Canvas::OnCommand(int commandId, int notifyCode)
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

    // MOUSE LEFT BUTTON DOWN
    case WM_LBUTTONDOWN: {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);

        pCanvas->HandleDraw(wParam, lParam);

        return 0;
    }

    // MOUSE MOVE
    case WM_MOUSEMOVE: {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);

        if (wParam & MK_LBUTTON)
        { // Рисуем только если зажата ЛКМ
            pCanvas->HandleDraw(wParam, lParam);
        }

        return 0;
    }

    // MOUSE UP
    case WM_LBUTTONUP: {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);

        return 0;
    }

    // MOUSE WHEEL
    case WM_MOUSEWHEEL: {

        return 0;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // Получаем размеры клиентской области
        RECT clientRect;
        GetClientRect(hWnd, &clientRect);
        int width = clientRect.right - clientRect.left;
        int height = clientRect.bottom - clientRect.top;

        // Создаём DC и bitmap в памяти
        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hMemBmp = CreateCompatibleBitmap(hdc, width, height);
        HGDIOBJ oldBmp = SelectObject(hdcMem, hMemBmp);

        // Очищаем фоном окна
        HBRUSH hBrush = GetSysColorBrush(COLOR_3DFACE);
        FillRect(hdcMem, &clientRect, hBrush);

        // Рисуем шахматный фон
        pCanvas->_chessBackground->Render(ps, hdcMem, pCanvas->_checkerSize);

        // Рисуем кадр с прозрачностью
        if (!pCanvas->_appState.frames.empty())
        {
            const Frame &frame = pCanvas->_appState.frames[0];

            BITMAPINFO bmi = {};
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = frame.width;
            bmi.bmiHeader.biHeight = -frame.height; // Top-down
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 32;
            bmi.bmiHeader.biCompression = BI_RGB;

            int logicalW = frame.width;
            int logicalH = frame.height;
            int screenW = logicalW * pCanvas->_checkerSize;
            int screenH = logicalH * pCanvas->_checkerSize;

            // Создаём временный DC для кадра
            HDC hdcFrame = CreateCompatibleDC(hdcMem);
            HBITMAP hFrameBmp = CreateCompatibleBitmap(hdcMem, logicalW, logicalH);
            HGDIOBJ oldFrameBmp = SelectObject(hdcFrame, hFrameBmp);

            // Рисуем кадр во временный DC
            SetDIBitsToDevice(hdcFrame, 0, 0, logicalW, logicalH, 0, 0, 0, logicalH, frame.pixels.get(), &bmi, DIB_RGB_COLORS);

            // Настраиваем альфа-смешивание
            BLENDFUNCTION blend = {};
            blend.BlendOp = AC_SRC_OVER;
            blend.BlendFlags = 0;
            blend.SourceConstantAlpha = 255;  // Полная непрозрачность
            blend.AlphaFormat = AC_SRC_ALPHA; // Используем альфа-канал из пикселей

            // Растягиваем с альфа-смешиванием (прозрачные пиксели не перекроют шахматку)
            AlphaBlend(hdcMem, 0, 0, screenW, screenH, hdcFrame, 0, 0, logicalW, logicalH, blend);

            // Очистка временного DC
            SelectObject(hdcFrame, oldFrameBmp);
            DeleteObject(hFrameBmp);
            DeleteDC(hdcFrame);
        }

        // 5. Копируем на экран
        BitBlt(hdc, 0, 0, width, height, hdcMem, 0, 0, SRCCOPY);

        // 6. Очистка
        SelectObject(hdcMem, oldBmp);
        DeleteObject(hMemBmp);
        DeleteDC(hdcMem);

        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_ERASEBKGND:
        // Preventing flickering
        return TRUE;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void Canvas::HandleDraw(WPARAM wParam, LPARAM lParam)
{
    if (_appState.frames.empty())
        return;

    // Пока работаем с первым кадром
    Frame &frame = _appState.frames[0];

    int mx = GET_X_LPARAM(lParam);
    int my = GET_Y_LPARAM(lParam);

    // Экран -> Логические координаты (делим на zoom)
    int lx = mx / _checkerSize;
    int ly = my / _checkerSize;

    // Проверка границ холста
    if (lx >= 0 && lx < frame.width && ly >= 0 && ly < frame.height)
    {
        uint32_t color = 0x00000000;

        if (_appState.currentTool == ToolType::Brush)
        {
            COLORREF c = _appState.palette.color;
            // COLORREF (0x00BBGGRR) → 0xAARRGGBB для GDI
            color = 0xFF000000 | (GetRValue(c) << 16) | (GetGValue(c) << 8) | GetBValue(c);
        }
        else if (_appState.currentTool == ToolType::Eraser)
        {
            color = 0x00000000; // Полностью прозрачный
        }

        // Меняем пиксель в памяти
        frame.SetPixel(lx, ly, color);

        // Перерисовываем ТОЛЬКО этот пиксель (оптимизация)
        RECT r = {lx * _checkerSize, ly * _checkerSize, (lx + 1) * _checkerSize, (ly + 1) * _checkerSize};
        InvalidateRect(_hCanvas, &r, FALSE); // FALSE = не стирать фон, используем двойную буферизацию
    }
}

bool Canvas::ZoomIn()
{
    if (_zoom < 3)
    { // Максимальный зум
        _zoom++;

        _canvasWidth = MIN_CANVAS_SIZE * _zoom;
        _canvasHeight = MIN_CANVAS_SIZE * _zoom;
        _checkerSize = _appState.checkerSize * _zoom;

        // Здесь можно добавить логику центрирования при зуме, если нужно
        ApplyZoom();

        std::cout << "ZoomIn" << std::endl;

        return true;
    }

    return false;
}

bool Canvas::ZoomOut()
{
    if (_zoom > 1)
    { // Минимальный зум
        _zoom--;

        _canvasWidth = MIN_CANVAS_SIZE * _zoom;
        _canvasHeight = MIN_CANVAS_SIZE * _zoom;
        _checkerSize = _appState.checkerSize * _zoom;

        ApplyZoom();

        std::cout << "ZoomOut" << std::endl;

        return true;
    }

    return false;
}

void Canvas::ApplyZoom()
{
    // Рассчитываем новый физический размер

    _canvasWidth = MIN_CANVAS_SIZE * _zoom;
    _canvasHeight = MIN_CANVAS_SIZE * _zoom;

    _offsetX = (_canvasAreaW - _canvasWidth) / 2;
    _offsetY = (_canvasAreaH - _canvasHeight) / 2;

    SetWindowPos(_hCanvas, nullptr, _offsetX, _offsetY, _canvasWidth, _canvasHeight, SWP_NOZORDER | SWP_NOACTIVATE);
}

int Canvas::GetZoom() const
{
    return _zoom;
}

HWND Canvas::GetHWndCanvas() const
{
    return _hCanvas;
}

} // namespace baresprite
