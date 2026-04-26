#pragma once
#include "AppState.h"
#include "ChildWindow.h"
#include <Windows.h>
#include <memory>

namespace baresprite
{

// Forward declaration
class ChessBackground;

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

    // Canvas Components
    std::unique_ptr<ChessBackground> _chessBackground;

    // Configuration
    static constexpr int LEFT_TOOLBAR_WIDTH = 180;
    static constexpr int RIGHT_TOOLBAR_WIDTH = 180;
    static constexpr int FRAME_TOOLBAR_HEIGHT = 80;

    // WndProc for Canvas
    static LRESULT CALLBACK _CanvasWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    void HandleDraw(WPARAM wParam, LPARAM lParam);
};

} // namespace baresprite