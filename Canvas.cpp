#include "Canvas.h"
#include "ChessBackground.h"
#include "ClipboardService.h"
#include "CursorRenderer.h"
#include "Frame.h"
#include "FrameRenderer.h"
#include "OnionFrameRenderer.h"
#include "SelectionRenderer.h"
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
    _frameRenderer = std::make_unique<FrameRenderer>();
    _cursorRenderer = std::make_unique<CursorRenderer>();
    _onionFrameRenderer = std::make_unique<OnionFrameRenderer>();
    _selectionRenderer = std::make_unique<SelectionRenderer>();
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

void Canvas::SetCustomCursor(bool isCustom)
{
    if (isCustom == _showCustomCursor)
    {
        return; // Ничего не меняем, если состояние то же самое
    }

    _showCustomCursor = isCustom;

    if (!isCustom)
    {
        // Скрываем курсор: стираем последнюю позицию
        if (_mousePosScreen.x >= 0 && _mousePosScreen.y >= 0)
        {
            int cursorSize = _brushSize * _checkerSize;
            int padding = 2; // Запас для рамки

            RECT eraseRect = {_mousePosScreen.x - cursorSize / 2 - padding, _mousePosScreen.y - cursorSize / 2 - padding,
                              _mousePosScreen.x + cursorSize / 2 + padding, _mousePosScreen.y + cursorSize / 2 + padding};

            // FALSE = не стирать фон, перерисуем через двойной буфер
            InvalidateRect(_hCanvas, &eraseRect, FALSE);
        }
    }
}

void Canvas::HandleDraw(WPARAM wParam, LPARAM lParam)
{

    if (_appState.frames.empty())
        return;

    Frame &frame = _appState.frames[_appState.currentFrameIndex];

    int mx = GET_X_LPARAM(lParam);
    int my = GET_Y_LPARAM(lParam);

    // Экран → логические координаты (центр кисти)
    int centerX = mx / _checkerSize;
    int centerY = my / _checkerSize;

    // Определяем цвет
    uint32_t color = 0x00000000;
    if (_appState.currentTool == ToolType::Brush)
    {
        COLORREF c = _appState.palette.color;
        color = 0xFF000000 | (GetRValue(c) << 16) | (GetGValue(c) << 8) | GetBValue(c);
    }
    else if (_appState.currentTool == ToolType::Eraser)
    {
        color = 0x00000000;
    }

    // Рисуем квадрат _brushSize × _brushSize вокруг центра
    int radius = _brushSize / 2; // Для 1→0, 3→1, 5→2

    // Собираем область для перерисовки (оптимизация)
    RECT dirtyRect = {(centerX - radius) * _checkerSize, (centerY - radius) * _checkerSize, (centerX + radius + 1) * _checkerSize,
                      (centerY + radius + 1) * _checkerSize};

    for (int dy = -radius; dy <= radius; ++dy)
    {
        for (int dx = -radius; dx <= radius; ++dx)
        {
            int lx = centerX + dx;
            int ly = centerY + dy;

            // Проверка границ холста
            if (lx >= 0 && lx < frame.width && ly >= 0 && ly < frame.height)
            {
                frame.SetPixel(lx, ly, color);
            }
        }
    }

    // Перерисовываем всю область кисти
    InvalidateRect(_hCanvas, &dirtyRect, FALSE);

    _appState.isDirty = true;
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

void Canvas::IncreaseBrushSize()
{
    const int MAX_BRUSH_SIZE = 5;

    if (_brushSize < MAX_BRUSH_SIZE)
    {
        int oldSize = _brushSize; // Запоминаем старый размер
        _brushSize += 2;
        InvalidateCursorArea(oldSize);
    }
}

void Canvas::DecreaseBrushSize()
{
    const int MIN_BRUSH_SIZE = 1;

    if (_brushSize > MIN_BRUSH_SIZE)
    {
        int oldSize = _brushSize; // Запоминаем старый размер
        _brushSize -= 2;
        InvalidateCursorArea(oldSize);
    }
}

void Canvas::InvalidateCursorArea(int oldSize) const
{
    if (!_showCustomCursor || _mousePosScreen.x < 0)
    {
        return; // Если курсор скрыт или не в холсте, не перерисовываем
    }

    // Находим максимальный размер, чтобы стереть "хвост" от старого курсора
    // и нарисовать новый.
    int maxDim = (oldSize > _brushSize) ? oldSize : _brushSize;

    int cursorSize = maxDim * _checkerSize;
    int padding = 2; // Запас для рамки

    // Вычисляем область, которая покрывает и старый, и новый курсор
    RECT dirtyRect = {_mousePosScreen.x - cursorSize / 2 - padding, _mousePosScreen.y - cursorSize / 2 - padding, _mousePosScreen.x + cursorSize / 2 + padding,
                      _mousePosScreen.y + cursorSize / 2 + padding};

    // FALSE = не стирать фон (предотвращаем мерцание)
    InvalidateRect(_hCanvas, &dirtyRect, FALSE);
}

void Canvas::InvalidateCursorArea() const
{
    if (!_showCustomCursor || _mousePosScreen.x < 0)
        return;

    int cursorSize = _brushSize * _checkerSize;
    int padding = 2;

    RECT dirtyRect = {_mousePosScreen.x - cursorSize / 2 - padding, _mousePosScreen.y - cursorSize / 2 - padding, _mousePosScreen.x + cursorSize / 2 + padding,
                      _mousePosScreen.y + cursorSize / 2 + padding};

    InvalidateRect(_hCanvas, &dirtyRect, FALSE);
}

/// <summary>
/// Load Frame
/// </summary>
/// <param name="frame"></param>
void Canvas::LoadFrame(const Frame &frame) const
{
    if (_hCanvas)
    {

        InvalidateRect(_hCanvas, nullptr, TRUE);
    }
}

void Canvas::ShiftCursorPos(int dx, int dy)
{
    if (!_showCustomCursor)
        return;

    // Сдвигаем сохранённые клиентские координаты
    _mousePosScreen.x += dx;
    _mousePosScreen.y += dy;

    // Перерисовываем только область курсора (без стирания фона)
    InvalidateCursorArea();
}

bool Canvas::GetShowCustomCursor() const
{
    return _showCustomCursor;
}

POINT Canvas::GetMousePosScreen() const
{
    return _mousePosScreen;
}

void Canvas::OnToolChanged(ToolType newTool)
{
    // Если переключились с Select на другой инструмент → сбрасываем выделение
    if (newTool != ToolType::Select && _appState.selection.isActive)
    {
        _appState.selection.Clear();
        InvalidateRect(_hCanvas, nullptr, FALSE);
    }
}

void Canvas::OnCut()
{
    ClipboardService::CopySelectionToClipboard(_appState, true);
}

void Canvas::OnCopy()
{
    ClipboardService::CopySelectionToClipboard(_appState, false);
}

void Canvas::OnPaste()
{
    ClipboardService::PasteFromClipboard(_appState);
}

void Canvas::OnUndo()
{
    _appState.history.Undo(_appState.frames);
    InvalidateRect(_hCanvas, nullptr, FALSE);
}

void Canvas::OnRedo()
{
    _appState.history.Redo(_appState.frames);
    InvalidateRect(_hCanvas, nullptr, FALSE);
}

/// <summary>
/// Window function
/// </summary>
/// <param name="hWnd"></param>
/// <param name="message"></param>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
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

        pCanvas->_appState.history.Commit(pCanvas->_appState.frames);

        // Processing for the Select tool
        if (pCanvas->_appState.currentTool == ToolType::Select)
        {
            SetFocus(pCanvas->_hCanvas);

            // Конвертация: экран → логические координаты кадра
            int logX = x / pCanvas->_checkerSize;
            int logY = y / pCanvas->_checkerSize;

            // Начинаем новое выделение
            pCanvas->_appState.selection.x = logX;
            pCanvas->_appState.selection.y = logY;
            pCanvas->_appState.selection.w = 0;
            pCanvas->_appState.selection.h = 0;
            pCanvas->_appState.selection.isDragging = true;
            pCanvas->_appState.selection.isActive = true;

            // Перерисовываем для показа "призрачного" прямоугольника
            InvalidateRect(pCanvas->_hCanvas, nullptr, FALSE);
            return 0;
        }

        pCanvas->HandleDraw(wParam, lParam);

        return 0;
    }

    // MOUSE MOVE
    case WM_MOUSEMOVE: {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);

        // Экран → логические координаты
        int logX = x / pCanvas->_checkerSize;
        int logY = y / pCanvas->_checkerSize;

        // Для HandleDraw
        pCanvas->_mousePos = {logX, logY};

        // Драг выделения для Select
        if (pCanvas->_appState.currentTool == ToolType::Select && pCanvas->_appState.selection.isDragging)
        {
            // Вычисляем размеры
            pCanvas->_appState.selection.w = logX - pCanvas->_appState.selection.x;
            pCanvas->_appState.selection.h = logY - pCanvas->_appState.selection.y;

            // Перерисовываем только область выделения (оптимизация)
            InvalidateRect(pCanvas->_hCanvas, nullptr, FALSE);
            return 0;
        }

        if (wParam & MK_LBUTTON)
        { // Рисуем только если зажата ЛКМ
            pCanvas->HandleDraw(wParam, lParam);
        }

        // Обновление курсора
        if (pCanvas->_showCustomCursor)
        {
            int cursorSize = pCanvas->_brushSize * pCanvas->_checkerSize;
            int padding = 2; // Запас для 1px рамки и избежания артефактов на краях

            // 1. Область СТАРОГО курсора (до обновления позиции)
            RECT oldRect = {pCanvas->_mousePosScreen.x - cursorSize / 2 - padding, pCanvas->_mousePosScreen.y - cursorSize / 2 - padding,
                            pCanvas->_mousePosScreen.x + cursorSize / 2 + padding, pCanvas->_mousePosScreen.y + cursorSize / 2 + padding};

            // 2. Обновляем позицию
            pCanvas->_mousePosScreen = {x, y};

            // 3. Область НОВОГО курсора
            RECT newRect = {x - cursorSize / 2 - padding, y - cursorSize / 2 - padding, x + cursorSize / 2 + padding, y + cursorSize / 2 + padding};

            // 4. Объединяем области: Windows перерисует только этот прямоугольник
            RECT dirtyRect;
            UnionRect(&dirtyRect, &oldRect, &newRect);

            // FALSE = не стирать фон (у нас двойная буферизация, фон рисуется в WM_PAINT)
            InvalidateRect(pCanvas->_hCanvas, &dirtyRect, FALSE);
        }

        return 0;
    }

    // MOUSE UP
    case WM_LBUTTONUP: {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);

        // Завершение выделения для Select
        if (pCanvas->_appState.currentTool == ToolType::Select && pCanvas->_appState.selection.isDragging)
        {
            pCanvas->_appState.selection.isDragging = false;

            // Нормализуем прямоугольник (на случай драга влево/вверх)
            pCanvas->_appState.selection.Normalize();

            // Перерисовываем для финального отображения
            InvalidateRect(pCanvas->_hCanvas, nullptr, FALSE);
            return 0;
        }

        return 0;
    }

    case WM_SETCURSOR: {
        // Если курсор над клиентской областью — скрываем системный
        if (LOWORD(lParam) == HTCLIENT)
        {

            if (pCanvas->_appState.currentTool == ToolType::Select)
            {
                // Для Select: показываем системный крестик
                SetCursor(LoadCursor(nullptr, IDC_CROSS));

                pCanvas->SetCustomCursor(false); // Скрываем кастомный квадрат
            }
            else
            {
                SetCursor(nullptr); // Скрыть стандартную стрелку/крест

                pCanvas->SetCustomCursor(true);
            }

            return TRUE;
        }
    }

    break;

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

        // Onion Skinning (предыдущие кадры)
        if (pCanvas->_appState.onionSkinEnabled && !pCanvas->_appState.frames.empty())
        {
            int currentIndex = pCanvas->_appState.currentFrameIndex;
            int total = static_cast<int>(pCanvas->_appState.frames.size());
            float baseOp = pCanvas->_appState.onionSkinOpacity;

            // Предыдущие кадры (затухание прозрачности)
            for (int i = 1; i <= pCanvas->_appState.onionSkinPrevFrames; ++i)
            {
                int index = currentIndex - i;
                if (index >= 0)
                {
                    float fade = 1.0f - (static_cast<float>(i - 1) / pCanvas->_appState.onionSkinPrevFrames);
                    const Frame &frame = pCanvas->_appState.frames[index];

                    pCanvas->_onionFrameRenderer->Render(index, baseOp * fade, frame, pCanvas->_checkerSize, hdcMem);
                }
            }

            // Следующие кадры (фиксированная прозрачность)
            for (int i = 1; i <= pCanvas->_appState.onionSkinNextFrames; ++i)
            {
                int index = currentIndex + i;
                if (index < total)
                {
                    const Frame &frame = pCanvas->_appState.frames[index];

                    pCanvas->_onionFrameRenderer->Render(index, baseOp * 0.5f, frame, pCanvas->_checkerSize, hdcMem);
                }
            }
        }

        // Рисуем кадр с прозрачностью
        if (!pCanvas->_appState.frames.empty())
        {
            const Frame &frame = pCanvas->_appState.frames[pCanvas->_appState.currentFrameIndex];
            pCanvas->_frameRenderer->Render(frame, pCanvas->_checkerSize, hdcMem);
        }

        // Рисуем кастомный курсор
        if (pCanvas->_showCustomCursor && pCanvas->_mousePos.x >= 0 && pCanvas->_mousePos.y >= 0)
        {
            int cursorSize = pCanvas->_brushSize * pCanvas->_checkerSize;
            int cursorX = pCanvas->_mousePosScreen.x - cursorSize / 2;
            int cursorY = pCanvas->_mousePosScreen.y - cursorSize / 2;

            pCanvas->_cursorRenderer->Render(cursorSize, cursorX, cursorY, hdcMem);
        }

        //  Рисуем рамку выделения (поверх кадра, под курсором)
        if (pCanvas->_appState.selection.isActive && pCanvas->_appState.currentTool == ToolType::Select)
        {
            pCanvas->_selectionRenderer->Render(hdcMem, pCanvas->_appState.selection, pCanvas->_checkerSize);
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

    case WM_KEYDOWN: {

        // Проверяем зажат ли Ctrl
        if (GetKeyState(VK_CONTROL) & 0x8000)
        {
            switch (wParam)
            {
            case 'C':

                ClipboardService::CopySelectionToClipboard(pCanvas->_appState, false);

                return 0;

            case 'X':

                ClipboardService::CopySelectionToClipboard(pCanvas->_appState, true);
                return 0;

            case 'V':

                ClipboardService::PasteFromClipboard(pCanvas->_appState);

                return 0;
            }
        }

        if (wParam == VK_ESCAPE && pCanvas->_appState.selection.isActive)
        {
            // Сбрасываем выделение
            pCanvas->_appState.selection.Clear();
            pCanvas->_appState.clipboard.Clear(); // Freeing up allocated RAM

            // Перерисовываем область, где была рамка
            InvalidateRect(pCanvas->_hCanvas, nullptr, FALSE);
            return 0;
        }
    }

    break;

    case WM_ERASEBKGND:
        // Preventing flickering
        return TRUE;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

} // namespace baresprite
