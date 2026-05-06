#include "Canvas.h"
#include "ChessBackground.h"
#include "ClipboardService.h"
#include "CursorRenderer.h"
#include "FillService.h"
#include "Frame.h"
#include "FrameRenderer.h"
#include "MirrorService.h"
#include "MoveDrag.h"
#include "MoveRenderer.h"
#include "OnionFrameRenderer.h"
#include "RotateService.h"
#include "SelectionRenderer.h"
#include "resource.h"
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
        return;
    }

    _checkerSize = _appState.checkerSize;

    _chessBackground = std::make_unique<ChessBackground>(appState);
    _frameRenderer = std::make_unique<FrameRenderer>();
    _cursorRenderer = std::make_unique<CursorRenderer>();
    _onionFrameRenderer = std::make_unique<OnionFrameRenderer>();
    _selectionRenderer = std::make_unique<SelectionRenderer>();
    _moveRenderer = std::make_unique<MoveRenderer>();
    _moveDrag = std::make_unique<MoveDrag>();
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
        return; // We don't change anything if the condition is the same.
    }

    _showCustomCursor = isCustom;

    if (!isCustom)
    {
        // Hiding the cursor: erasing the last position
        if (_mousePosScreen.x >= 0 && _mousePosScreen.y >= 0)
        {
            int cursorSize = _brushSize * _checkerSize;
            int padding = 2; // Frame reserve

            RECT eraseRect = {_mousePosScreen.x - cursorSize / 2 - padding, _mousePosScreen.y - cursorSize / 2 - padding,
                              _mousePosScreen.x + cursorSize / 2 + padding, _mousePosScreen.y + cursorSize / 2 + padding};

            // FALSE = Don't erase the background, redraw it using a double buffer
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

    // Screen -> Logical Coordinates (Brush Center)
    int centerX = mx / _checkerSize;
    int centerY = my / _checkerSize;

    // Determine the color
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

    // Draw a square _brushSize × _brushSize around the center
    int radius = _brushSize / 2; // Для 1->0, 3->1, 5->2

    // Collecting the area to be redrawed (optimization)
    RECT dirtyRect = {(centerX - radius) * _checkerSize, (centerY - radius) * _checkerSize, (centerX + radius + 1) * _checkerSize,
                      (centerY + radius + 1) * _checkerSize};

    for (int dy = -radius; dy <= radius; ++dy)
    {
        for (int dx = -radius; dx <= radius; ++dx)
        {
            int lx = centerX + dx;
            int ly = centerY + dy;

            // Checking canvas boundaries
            if (lx >= 0 && lx < frame.width && ly >= 0 && ly < frame.height)
            {
                frame.SetPixel(lx, ly, color);
            }
        }
    }

    // Redraw the entire brush area
    InvalidateRect(_hCanvas, &dirtyRect, FALSE);

    _appState.isDirty = true;
}

bool Canvas::ZoomIn()
{
    if (_zoom < 3)
    { // Maximum zoom
        _zoom++;

        _canvasWidth = MIN_CANVAS_SIZE * _zoom;
        _canvasHeight = MIN_CANVAS_SIZE * _zoom;
        _checkerSize = _appState.checkerSize * _zoom;

        ApplyZoom();

        return true;
    }

    return false;
}

bool Canvas::ZoomOut()
{
    if (_zoom > 1)
    { // Minimum zoom
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
    // Calculating the new physical size

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
        int oldSize = _brushSize; // Remember the old size
        _brushSize += 2;
        InvalidateCursorArea(oldSize);
    }
}

void Canvas::DecreaseBrushSize()
{
    const int MIN_BRUSH_SIZE = 1;

    if (_brushSize > MIN_BRUSH_SIZE)
    {
        int oldSize = _brushSize; // Remember the old size
        _brushSize -= 2;
        InvalidateCursorArea(oldSize);
    }
}

void Canvas::InvalidateCursorArea(int oldSize) const
{
    if (!_showCustomCursor || _mousePosScreen.x < 0)
    {
        return; // If the cursor is hidden or not in the canvas, do not redraw
    }

    // We find the maximum size to erase the "tail" of the old cursor and draw a new one.
    int maxDim = (oldSize > _brushSize) ? oldSize : _brushSize;

    int cursorSize = maxDim * _checkerSize;
    int padding = 2; // Запас для рамки

    // We calculate the area that covers both the old and new cursors
    RECT dirtyRect = {_mousePosScreen.x - cursorSize / 2 - padding, _mousePosScreen.y - cursorSize / 2 - padding, _mousePosScreen.x + cursorSize / 2 + padding,
                      _mousePosScreen.y + cursorSize / 2 + padding};

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

    _mousePosScreen.x += dx;
    _mousePosScreen.y += dy;

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

            int logX = x / pCanvas->_checkerSize;
            int logY = y / pCanvas->_checkerSize;

            // We save the starting point separately
            pCanvas->_appState.selection.startX = logX;
            pCanvas->_appState.selection.startY = logY;

            // Let's start a new selection
            pCanvas->_appState.selection.x = logX;
            pCanvas->_appState.selection.y = logY;
            pCanvas->_appState.selection.w = 0;
            pCanvas->_appState.selection.h = 0;
            pCanvas->_appState.selection.isDragging = true;
            pCanvas->_appState.selection.isActive = true;

            InvalidateRect(pCanvas->_hCanvas, nullptr, FALSE);
            return 0;
        }

        else if (pCanvas->_appState.currentTool == ToolType::Fill)
        {

            int logX = x / pCanvas->_checkerSize;
            int logY = y / pCanvas->_checkerSize;

            if (logX >= 0 && logY >= 0 && !pCanvas->_appState.frames.empty())
            {
                Frame &frame = pCanvas->_appState.frames[pCanvas->_appState.currentFrameIndex];

                // Save state for UNDO
                pCanvas->_appState.history.Commit(pCanvas->_appState.frames, pCanvas->_appState.selection.x, pCanvas->_appState.selection.y,
                                                  pCanvas->_appState.selection.w, pCanvas->_appState.selection.h, pCanvas->_appState.selection.isActive);

                // We get a color from the palette (convert COLORREF -> 0xAARRGGBB)
                COLORREF palColor = pCanvas->_appState.palette.color;
                uint32_t fillColor = 0xFF000000 | (static_cast<uint32_t>(GetRValue(palColor)) << 16) | (static_cast<uint32_t>(GetGValue(palColor)) << 8) |
                                     static_cast<uint32_t>(GetBValue(palColor));

                // Let's start the filling
                FillService::PerformFill(frame, pCanvas->_appState.selection, logX, logY, fillColor);

                pCanvas->_appState.isDirty = true;

                InvalidateRect(pCanvas->_hCanvas, nullptr, FALSE);
            }
            return 0;
        }

        else if (pCanvas->_appState.currentTool == ToolType::Move)
        {
            // Move drag (down mouse left)
            pCanvas->_moveDrag->ButtonDown(pCanvas->_appState, pCanvas->_hCanvas, x, y, pCanvas->_checkerSize);

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

        int logX = x / pCanvas->_checkerSize;
        int logY = y / pCanvas->_checkerSize;

        // Для HandleDraw
        pCanvas->_mousePos = {logX, logY};

        // Drag selection for Select
        if (pCanvas->_appState.currentTool == ToolType::Select && pCanvas->_appState.selection.isDragging)
        {
            // Reading the fixed start
            int startX = pCanvas->_appState.selection.startX;
            int startY = pCanvas->_appState.selection.startY;

            // Calculating the boundaries
            int left = min(startX, logX);
            int top = min(startY, logY);
            int right = max(startX, logX);
            int bottom = max(startY, logY);

            // Update x,y,w,h for rendering
            pCanvas->_appState.selection.x = left;
            pCanvas->_appState.selection.y = top;
            pCanvas->_appState.selection.w = right - left + 1;
            pCanvas->_appState.selection.h = bottom - top + 1;

            // Redraw only the selection area (optimization)
            InvalidateRect(pCanvas->_hCanvas, nullptr, FALSE);
            return 0;
        }

        if ((wParam & MK_LBUTTON) && (pCanvas->_appState.currentTool == ToolType::Brush || pCanvas->_appState.currentTool == ToolType::Eraser))
        { // Draw only if the LMB is pressed
            pCanvas->HandleDraw(wParam, lParam);
        }

        // Cursor update
        if (pCanvas->_showCustomCursor)
        {
            int cursorSize = pCanvas->_brushSize * pCanvas->_checkerSize;
            int padding = 2;

            // Old cursor area (before position update)
            RECT oldRect = {pCanvas->_mousePosScreen.x - cursorSize / 2 - padding, pCanvas->_mousePosScreen.y - cursorSize / 2 - padding,
                            pCanvas->_mousePosScreen.x + cursorSize / 2 + padding, pCanvas->_mousePosScreen.y + cursorSize / 2 + padding};

            // Updating the position
            pCanvas->_mousePosScreen = {x, y};

            // New cursor area
            RECT newRect = {x - cursorSize / 2 - padding, y - cursorSize / 2 - padding, x + cursorSize / 2 + padding, y + cursorSize / 2 + padding};

            // Combining areas
            RECT dirtyRect;
            UnionRect(&dirtyRect, &oldRect, &newRect);

            InvalidateRect(pCanvas->_hCanvas, &dirtyRect, FALSE);
        }

        // Drag logic for Move
        if (pCanvas->_appState.currentTool == ToolType::Move && pCanvas->_appState.moveDrag.isDragging)
        {
            // Move drag (move mouse left)
            pCanvas->_moveDrag->ButtonMove(pCanvas->_appState, pCanvas->_hCanvas, x, y, pCanvas->_checkerSize);

            return 0;
        }

        return 0;
    }

    // MOUSE UP
    case WM_LBUTTONUP: {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);

        // Completing the selection for Select
        if (pCanvas->_appState.currentTool == ToolType::Select && pCanvas->_appState.selection.isDragging)
        {
            pCanvas->_appState.selection.isDragging = false;

            // Normalize the rectangle (in case of dragging to the left/up)
            pCanvas->_appState.selection.Normalize();

            // Cleaning startX/startY
            pCanvas->_appState.selection.startX = 0;
            pCanvas->_appState.selection.startY = 0;

            // Redrawing for final display
            InvalidateRect(pCanvas->_hCanvas, nullptr, FALSE);
            return 0;
        }

        // Finish dragging - Move
        if (pCanvas->_appState.currentTool == ToolType::Move && pCanvas->_appState.moveDrag.isDragging)
        {
            pCanvas->_moveDrag->ButtonUp(pCanvas->_appState, pCanvas->_hCanvas);

            pCanvas->_appState.isDirty = true;

            return 0;
        }

        return 0;
    }

    case WM_SETCURSOR: {
        // If the cursor is over the client area, hide the system
        if (LOWORD(lParam) == HTCLIENT)
        {

            // List of tools that use system cursors
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
                    hCur = LoadCursor(nullptr, IDC_ARROW); // Cursor for Rotate and Mirror
                }

                else
                {
                    hCur = LoadCursor(pCanvas->_hInstanceParent, MAKEINTRESOURCE(IDC_CUSTOM_CROSS_CURSOR));
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

        // Getting the dimensions of the client area
        RECT clientRect;
        GetClientRect(hWnd, &clientRect);
        int width = clientRect.right - clientRect.left;
        int height = clientRect.bottom - clientRect.top;

        // Create a DC and bitmap in memory
        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hMemBmp = CreateCompatibleBitmap(hdc, width, height);
        HGDIOBJ oldBmp = SelectObject(hdcMem, hMemBmp);

        // Clearing the window background
        HBRUSH hBrush = GetSysColorBrush(COLOR_3DFACE);
        FillRect(hdcMem, &clientRect, hBrush);

        // Drawing a checkerboard background
        pCanvas->_chessBackground->Render(ps, hdcMem, pCanvas->_checkerSize);

        // Onion Skinning (previous frames)
        if (pCanvas->_appState.onionSkinEnabled && !pCanvas->_appState.frames.empty())
        {
            int currentIndex = pCanvas->_appState.currentFrameIndex;
            int total = static_cast<int>(pCanvas->_appState.frames.size());
            float baseOp = pCanvas->_appState.onionSkinOpacity;

            // Previous frames (transparency fading)
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

            // Next frames (fixed transparency)
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

        // Drawing a frame with transparency
        if (!pCanvas->_appState.frames.empty())
        {
            const Frame &frame = pCanvas->_appState.frames[pCanvas->_appState.currentFrameIndex];
            pCanvas->_frameRenderer->Render(frame, pCanvas->_checkerSize, hdcMem);

            // Draw a selection while dragging
            pCanvas->_moveRenderer->Render(frame, pCanvas->_checkerSize, pCanvas->_appState, hdcMem);
        }

        // Drawing a custom cursor
        if (pCanvas->_showCustomCursor && pCanvas->_mousePos.x >= 0 && pCanvas->_mousePos.y >= 0)
        {
            int cursorSize = pCanvas->_brushSize * pCanvas->_checkerSize;
            int cursorX = pCanvas->_mousePosScreen.x - cursorSize / 2;
            int cursorY = pCanvas->_mousePosScreen.y - cursorSize / 2;

            pCanvas->_cursorRenderer->Render(cursorSize, cursorX, cursorY, hdcMem);
        }

        //  Draw a selection frame (over the frame, under the cursor)
        if (pCanvas->_appState.selection.isActive && (pCanvas->_appState.currentTool == ToolType::Select || pCanvas->_appState.currentTool == ToolType::Fill))
        {
            pCanvas->_selectionRenderer->Render(hdcMem, pCanvas->_appState.selection, pCanvas->_checkerSize);
        }

        // Copy to screen
        BitBlt(hdc, 0, 0, width, height, hdcMem, 0, 0, SRCCOPY);

        // Cleaning
        SelectObject(hdcMem, oldBmp);
        DeleteObject(hMemBmp);
        DeleteDC(hdcMem);

        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_KEYDOWN: {

        // Check if Ctrl is pressed
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
            // Reset the selection
            pCanvas->_appState.selection.Clear();
            pCanvas->_appState.clipboard.Clear(); // Freeing up allocated RAM

            // Redraw the area where the frame was
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
