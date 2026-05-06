#pragma once
#include "AppState.h"
#include "ChildWindow.h"
#include <Windows.h>
#include <memory>

namespace baresprite
{

// Forward declaration
class ChessBackground;
class FrameRenderer;
class CursorRenderer;
class OnionFrameRenderer;
class SelectionRenderer;
class MoveRenderer;
class MoveDrag;

class Canvas : public ChildWindow
{
  public:
    Canvas(HWND hWndParent, HINSTANCE hInstanceParent, AppState &appState);

    ~Canvas() override;

    void OnSize(int clientW, int clientH) override;
    bool OnCommand(int commandId, int notifyCode) override;
    bool ZoomIn();
    bool ZoomOut();
    int GetZoom() const;
    void ApplyZoom();
    HWND GetHWndCanvas() const;
    void SetCustomCursor(bool isCustom);
    void IncreaseBrushSize();
    void DecreaseBrushSize();
    void InvalidateCursorArea(int oldSize) const;
    void InvalidateCursorArea() const;
    void LoadFrame(const Frame &frame) const;
    void ShiftCursorPos(int dx, int dy);
    bool GetShowCustomCursor() const;
    POINT GetMousePosScreen() const;
    void OnToolChanged(ToolType newTool);

    void OnCut();
    void OnCopy();
    void OnPaste();
    void OnUndo();
    void OnRedo();

    void OnRotateR();
    void OnRotateL();

    void OnMirrorHorizontal();
    void OnMirrorVertical();

  private:
    HWND _hWndParent;
    HINSTANCE _hInstanceParent;

    // Link to the project data structure.
    AppState &_appState;

    HWND _hCanvas = nullptr;

    int _canvasAreaW = 0;
    int _canvasAreaH = 0;

    static constexpr int MIN_CANVAS_SIZE = 512;

    // Canvas state
    int _canvasWidth = MIN_CANVAS_SIZE;
    int _canvasHeight = MIN_CANVAS_SIZE;
    int _checkerSize = 8;

    int _zoom = 1;
    int _offsetX = 0;
    int _offsetY = 0;

    // Custom cursor
    POINT _mousePosScreen = {-1, -1};
    POINT _mousePos = {-1, -1};
    POINT _prevMousePos = {-1, -1};
    int _brushSize = 1;
    bool _showCustomCursor = false;

    // Canvas Components
    std::unique_ptr<ChessBackground> _chessBackground;
    std::unique_ptr<FrameRenderer> _frameRenderer;
    std::unique_ptr<CursorRenderer> _cursorRenderer;
    std::unique_ptr<OnionFrameRenderer> _onionFrameRenderer;
    std::unique_ptr<SelectionRenderer> _selectionRenderer;
    std::unique_ptr<MoveRenderer> _moveRenderer;
    std::unique_ptr<MoveDrag> _moveDrag;

    // Configuration
    static constexpr int LEFT_TOOLBAR_WIDTH = 180;
    static constexpr int RIGHT_TOOLBAR_WIDTH = 180;
    static constexpr int FRAME_TOOLBAR_HEIGHT = 80;

    // WndProc for Canvas
    static LRESULT CALLBACK _CanvasWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    void HandleDraw(WPARAM wParam, LPARAM lParam);
};

} // namespace baresprite