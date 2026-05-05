#include "Canvas.h"
#include "ChessBackground.h"
#include "ClipboardService.h"
#include "CursorRenderer.h"
#include "FillService.h"
#include "Frame.h"
#include "FrameRenderer.h"
#include "MirrorService.h"
#include "OnionFrameRenderer.h"
#include "RotateService.h"
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
    if (newTool != ToolType::Select && newTool != ToolType::Fill && newTool != ToolType::Move && newTool != ToolType::RotateR && newTool != ToolType::RotateL &&
        newTool != ToolType::MirrorV && newTool != ToolType::MirrorH && _appState.selection.isActive)
    {
        _appState.selection.Clear();
        InvalidateRect(_hCanvas, nullptr, FALSE);
    }
}

void Canvas::OnCut()
{
    _appState.history.Commit(_appState.frames, _appState.selection.x, _appState.selection.y, _appState.selection.w, _appState.selection.h,
                             _appState.selection.isActive);

    ClipboardService::CopySelectionToClipboard(_appState, true);
    _appState.isDirty = true;
    InvalidateRect(_hCanvas, nullptr, FALSE);
}

void Canvas::OnCopy()
{
    _appState.history.Commit(_appState.frames, _appState.selection.x, _appState.selection.y, _appState.selection.w, _appState.selection.h,
                             _appState.selection.isActive);

    ClipboardService::CopySelectionToClipboard(_appState, false);
}

void Canvas::OnPaste()
{
    _appState.history.Commit(_appState.frames, _appState.selection.x, _appState.selection.y, _appState.selection.w, _appState.selection.h,
                             _appState.selection.isActive);

    ClipboardService::PasteFromClipboard(_appState);

    _appState.isDirty = true;
    InvalidateRect(_hCanvas, nullptr, FALSE);
}

void Canvas::OnUndo()
{
    _appState.history.Undo(_appState.frames, _appState.selection.x, _appState.selection.y, _appState.selection.w, _appState.selection.h,
                           _appState.selection.isActive);

    InvalidateRect(_hCanvas, nullptr, FALSE);
}

void Canvas::OnRedo()
{
    _appState.history.Redo(_appState.frames, _appState.selection.x, _appState.selection.y, _appState.selection.w, _appState.selection.h,
                           _appState.selection.isActive);

    InvalidateRect(_hCanvas, nullptr, FALSE);
}

void Canvas::OnRotateR()
{

    if (_appState.selection.isActive)
    {

        RotateService::RotateSelection90R(_appState, _hCanvas);
    }
}

void Canvas::OnRotateL()
{

    if (_appState.selection.isActive)
    {

        RotateService::RotateSelection90L(_appState, _hCanvas);
    }
}

void Canvas::OnMirrorHorizontal()
{
    if (_appState.selection.isActive)
    {
        MirrorService::FlipHorizontal(_appState, _hCanvas);
    }
}

void Canvas::OnMirrorVertical()
{
    if (_appState.selection.isActive)
    {
        MirrorService::FlipVertical(_appState, _hCanvas);
    }
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

        // Processing for the Select tool
        if (pCanvas->_appState.currentTool == ToolType::Select)
        {
            SetFocus(pCanvas->_hCanvas);

            // Конвертация: экран → логические координаты кадра
            int logX = x / pCanvas->_checkerSize;
            int logY = y / pCanvas->_checkerSize;

            // Сохраняем точку старта отдельно
            pCanvas->_appState.selection.startX = logX;
            pCanvas->_appState.selection.startY = logY;

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

        else if (pCanvas->_appState.currentTool == ToolType::Fill)
        {
            // 1. Конвертация: экран → логические координаты
            int logX = x / pCanvas->_checkerSize;
            int logY = y / pCanvas->_checkerSize;

            if (logX >= 0 && logY >= 0 && !pCanvas->_appState.frames.empty())
            {
                Frame &frame = pCanvas->_appState.frames[pCanvas->_appState.currentFrameIndex];

                // Save state for UNDO
                pCanvas->_appState.history.Commit(pCanvas->_appState.frames, pCanvas->_appState.selection.x, pCanvas->_appState.selection.y,
                                                  pCanvas->_appState.selection.w, pCanvas->_appState.selection.h, pCanvas->_appState.selection.isActive);

                // Получаем цвет из палитры (преобразуем COLORREF → 0xAARRGGBB)
                COLORREF palColor = pCanvas->_appState.palette.color;
                uint32_t fillColor = 0xFF000000 | (static_cast<uint32_t>(GetRValue(palColor)) << 16) | (static_cast<uint32_t>(GetGValue(palColor)) << 8) |
                                     static_cast<uint32_t>(GetBValue(palColor));

                // Запускаем заливку (с учётом выделения)
                FillService::PerformFill(frame, pCanvas->_appState.selection, logX, logY, fillColor);

                // Помечаем проект как изменённый и перерисовываем
                pCanvas->_appState.isDirty = true;

                InvalidateRect(pCanvas->_hCanvas, nullptr, FALSE);
            }
            return 0;
        }

        else if (pCanvas->_appState.currentTool == ToolType::Move)
        {

            int logX = x / pCanvas->_checkerSize;
            int logY = y / pCanvas->_checkerSize;
            const auto &sel = pCanvas->_appState.selection;

            if (sel.isActive && logX >= sel.x && logX < sel.x + sel.w && logY >= sel.y && logY < sel.y + sel.h)
            {
                Frame &frame = pCanvas->_appState.frames[pCanvas->_appState.currentFrameIndex];

                // Сохраняем для Undo
                pCanvas->_appState.history.Commit(pCanvas->_appState.frames, pCanvas->_appState.selection.x, pCanvas->_appState.selection.y,
                                                  pCanvas->_appState.selection.w, pCanvas->_appState.selection.h, pCanvas->_appState.selection.isActive);

                // Копируем пиксели в буфер
                pCanvas->_appState.moveDrag.pixelBuffer.resize(sel.w * sel.h);
                for (int dy = 0; dy < sel.h; ++dy)
                {
                    for (int dx = 0; dx < sel.w; ++dx)
                    {
                        pCanvas->_appState.moveDrag.pixelBuffer[dy * sel.w + dx] = frame.GetPixel(sel.x + dx, sel.y + dy);
                    }
                }

                // Очищаем выделение в кадре (делает дырку прозрачной)
                for (int dy = 0; dy < sel.h; ++dy)
                {
                    for (int dx = 0; dx < sel.w; ++dx)
                    {
                        frame.SetPixel(sel.x + dx, sel.y + dy, 0);
                    }
                }

                // Инициализируем драг
                pCanvas->_appState.moveDrag.isDragging = true;
                pCanvas->_appState.moveDrag.startLogX = logX;
                pCanvas->_appState.moveDrag.startLogY = logY;
                pCanvas->_appState.moveDrag.origSelX = sel.x;
                pCanvas->_appState.moveDrag.origSelY = sel.y;
                pCanvas->_appState.moveDrag.w = sel.w;
                pCanvas->_appState.moveDrag.h = sel.h;

                InvalidateRect(pCanvas->_hCanvas, nullptr, FALSE);
            }
            return 0;
        }

        else if (pCanvas->_appState.currentTool == ToolType::Brush || pCanvas->_appState.currentTool == ToolType::Eraser)
        {
            pCanvas->_appState.history.Commit(pCanvas->_appState.frames, pCanvas->_appState.selection.x, pCanvas->_appState.selection.y,
                                              pCanvas->_appState.selection.w, pCanvas->_appState.selection.h, pCanvas->_appState.selection.isActive);

            pCanvas->HandleDraw(wParam, lParam);
        }

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
            // Читаем фиксированный старт
            int startX = pCanvas->_appState.selection.startX;
            int startY = pCanvas->_appState.selection.startY;

            // Вычисляем границы
            int left = min(startX, logX);
            int top = min(startY, logY);
            int right = max(startX, logX);
            int bottom = max(startY, logY);

            // Обновляем x,y,w,h для отрисовки
            pCanvas->_appState.selection.x = left;
            pCanvas->_appState.selection.y = top;
            pCanvas->_appState.selection.w = right - left + 1;
            pCanvas->_appState.selection.h = bottom - top + 1;

            // Перерисовываем только область выделения (оптимизация)
            InvalidateRect(pCanvas->_hCanvas, nullptr, FALSE);
            return 0;
        }

        if ((wParam & MK_LBUTTON) && (pCanvas->_appState.currentTool == ToolType::Brush || pCanvas->_appState.currentTool == ToolType::Eraser))
        { // Рисуем только если зажата ЛКМ
            pCanvas->HandleDraw(wParam, lParam);
        }

        // Обновление курсора
        if (pCanvas->_showCustomCursor)
        {
            int cursorSize = pCanvas->_brushSize * pCanvas->_checkerSize;
            int padding = 2; // Запас для 1px рамки и избежания артефактов на краях

            // Область старого курсора (до обновления позиции)
            RECT oldRect = {pCanvas->_mousePosScreen.x - cursorSize / 2 - padding, pCanvas->_mousePosScreen.y - cursorSize / 2 - padding,
                            pCanvas->_mousePosScreen.x + cursorSize / 2 + padding, pCanvas->_mousePosScreen.y + cursorSize / 2 + padding};

            // Обновляем позицию
            pCanvas->_mousePosScreen = {x, y};

            // Область нового курсора
            RECT newRect = {x - cursorSize / 2 - padding, y - cursorSize / 2 - padding, x + cursorSize / 2 + padding, y + cursorSize / 2 + padding};

            // Объединяем области
            RECT dirtyRect;
            UnionRect(&dirtyRect, &oldRect, &newRect);

            InvalidateRect(pCanvas->_hCanvas, &dirtyRect, FALSE);
        }

        // Логика перетаскивания для Move
        if (pCanvas->_appState.currentTool == ToolType::Move && pCanvas->_appState.moveDrag.isDragging)
        {
            int logX = x / pCanvas->_checkerSize;
            int logY = y / pCanvas->_checkerSize;

            int dx = logX - pCanvas->_appState.moveDrag.startLogX;
            int dy = logY - pCanvas->_appState.moveDrag.startLogY;

            int newX = pCanvas->_appState.moveDrag.origSelX + dx;
            int newY = pCanvas->_appState.moveDrag.origSelY + dy;

            Frame &frame = pCanvas->_appState.frames[pCanvas->_appState.currentFrameIndex];
            int w = pCanvas->_appState.moveDrag.w;
            int h = pCanvas->_appState.moveDrag.h;

            // Ограничиваем границами холста
            if (newX < 0)
                newX = 0;
            if (newY < 0)
                newY = 0;
            if (newX + w > frame.width)
                newX = frame.width - w;
            if (newY + h > frame.height)
                newY = frame.height - h;

            // Обновляем координаты выделения (используем как превью-позицию)
            pCanvas->_appState.selection.x = newX;
            pCanvas->_appState.selection.y = newY;

            // Перерисовываем весь холст (фон + кадр + превью выделения)
            InvalidateRect(pCanvas->_hCanvas, nullptr, FALSE);
            return 0;
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

            // Очистка startX/startY
            pCanvas->_appState.selection.startX = 0;
            pCanvas->_appState.selection.startY = 0;

            // Перерисовываем для финального отображения
            InvalidateRect(pCanvas->_hCanvas, nullptr, FALSE);
            return 0;
        }

        // Завершение перетаскивания - Move
        if (pCanvas->_appState.currentTool == ToolType::Move && pCanvas->_appState.moveDrag.isDragging)
        {
            Frame &frame = pCanvas->_appState.frames[pCanvas->_appState.currentFrameIndex];
            const auto &drag = pCanvas->_appState.moveDrag;
            int finalX = pCanvas->_appState.selection.x;
            int finalY = pCanvas->_appState.selection.y;

            // Коммитим буфер в кадр на финальную позицию
            for (int cy = 0; cy < drag.h; ++cy)
            {
                for (int cx = 0; cx < drag.w; ++cx)
                {
                    int px = finalX + cx;
                    int py = finalY + cy;
                    if (px >= 0 && px < frame.width && py >= 0 && py < frame.height)
                    {
                        frame.SetPixel(px, py, drag.pixelBuffer[cy * drag.w + cx]);
                    }
                }
            }

            // Очищаем состояние драга
            pCanvas->_appState.moveDrag.isDragging = false;
            pCanvas->_appState.moveDrag.pixelBuffer.clear();
            pCanvas->_appState.moveDrag.pixelBuffer.shrink_to_fit();

            InvalidateRect(pCanvas->_hCanvas, nullptr, FALSE);
            return 0;
        }

        return 0;
    }

    case WM_SETCURSOR: {
        // Если курсор над клиентской областью — скрываем системный
        if (LOWORD(lParam) == HTCLIENT)
        {

            // Список инструментов, использующих системные курсоры
            bool useSystemCursor = (pCanvas->_appState.currentTool == ToolType::Select || pCanvas->_appState.currentTool == ToolType::Fill ||
                                    pCanvas->_appState.currentTool == ToolType::Move || pCanvas->_appState.currentTool == ToolType::RotateR ||
                                    pCanvas->_appState.currentTool == ToolType::RotateL);

            if (useSystemCursor)
            {
                HCURSOR hCur;
                if (pCanvas->_appState.currentTool == ToolType::Move)
                {
                    hCur = LoadCursor(nullptr, IDC_SIZEALL);
                }

                else if (pCanvas->_appState.currentTool == ToolType::RotateR || pCanvas->_appState.currentTool == ToolType::RotateL ||
                         pCanvas->_appState.currentTool == ToolType::MirrorV || pCanvas->_appState.currentTool == ToolType::MirrorH)
                {
                    hCur = LoadCursor(nullptr, IDC_ARROW); // Курсор для Rotate и Mirror
                }

                else
                {
                    hCur = LoadCursor(nullptr, IDC_ARROW);
                }

                SetCursor(hCur);
                pCanvas->SetCustomCursor(false);
            }
            else
            {
                SetCursor(nullptr);
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

            // Рисуем "летающее" выделение, если идёт перетаскивание
            if (pCanvas->_appState.moveDrag.isDragging)
            {
                const auto &drag = pCanvas->_appState.moveDrag;
                for (int cy = 0; cy < drag.h; ++cy)
                {
                    for (int cx = 0; cx < drag.w; ++cx)
                    {
                        int px = pCanvas->_appState.selection.x + cx;
                        int py = pCanvas->_appState.selection.y + cy;

                        if (px >= 0 && px < frame.width && py >= 0 && py < frame.height)
                        {
                            uint32_t pixel = drag.pixelBuffer[cy * drag.w + cx];
                            unsigned char a = (pixel >> 24) & 0xFF;
                            if (a > 0) // Не рисуем полностью прозрачные пиксели
                            {
                                COLORREF col = RGB((pixel >> 16) & 0xFF, (pixel >> 8) & 0xFF, pixel & 0xFF);
                                // Рисуем квадратик с учётом зума
                                for (int sy = 0; sy < pCanvas->_checkerSize; ++sy)
                                {
                                    for (int sx = 0; sx < pCanvas->_checkerSize; ++sx)
                                    {
                                        SetPixelV(hdcMem, px * pCanvas->_checkerSize + sx, py * pCanvas->_checkerSize + sy, col);
                                    }
                                }
                            }
                        }
                    }
                }
            }
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
        if (pCanvas->_appState.selection.isActive && (pCanvas->_appState.currentTool == ToolType::Select || pCanvas->_appState.currentTool == ToolType::Fill))
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

                pCanvas->OnCopy();
                return 0;

            case 'X':

                pCanvas->OnCut();
                return 0;

            case 'V':

                pCanvas->OnPaste();
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
